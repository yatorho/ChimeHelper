#ifndef BFC_ALLOCATOR_BFC_ALLOCATOR_H_
#define BFC_ALLOCATOR_BFC_ALLOCATOR_H_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <set>
#include <sstream>
#include <vector>

#include "chime/core/platform/logging.hpp"
#include "chime/core/platform/macros.h"

namespace memory {

class Allocator {
public:
  virtual ~Allocator() {}
  virtual void *Allocate(size_t size, size_t align) = 0;
  virtual void Deallocate(void *ptr) = 0;
};

class CPUAllocator : public Allocator {
public:
  ~CPUAllocator() override {}
  void *Allocate(size_t size, size_t align) override {
    return aligned_alloc(align, size);  // malloc
  }
  void Deallocate(void *ptr) override { free(ptr); }
};

// class SubAllocator {
//  public:
// };

class BFCAllocator : public Allocator {
public:
  BFCAllocator(Allocator *allocator, size_t memory_limit, bool allow_growth);

  ~BFCAllocator() override;

  void *Allocate(size_t size, size_t align) override;

  void Deallocate(void *ptr) override;

  static constexpr size_t MIN_ALLOCATION_BITS = 8;
  static constexpr size_t MIN_ALLOCATION_SIZE = 1 << MIN_ALLOCATION_BITS;

private:
  typedef int64_t ChunkHandle;
  typedef int BinNum;
  static constexpr BinNum NUM_BINS = 21;

  static constexpr ChunkHandle INVALID_CHUNK_HANDLE = SIZE_MAX;
  static constexpr BinNum INVALID_BIN_NUM = -1;

  struct Chunk;
  struct Bin;

  struct Chunk {
    size_t size = 0;
    size_t req_size = 0;
    int64_t allocation_id = -1;
    void *ptr = nullptr;

    ChunkHandle next = INVALID_CHUNK_HANDLE;
    ChunkHandle prev = INVALID_CHUNK_HANDLE;

    BinNum bin_num = INVALID_BIN_NUM;

    bool IsUse() const { return allocation_id != -1; }

    std::string DebugString() const {
      std::stringstream ss;
      ss << "Chunk " << ptr << " | size: " << size
         << " | req_size: " << req_size << " | alloc_id: " << allocation_id
         << " | bin_num: " << bin_num;
      if (prev != INVALID_CHUNK_HANDLE) {
        ss << " | prev: " << prev;
      }
      if (next != INVALID_CHUNK_HANDLE) {
        ss << " | next: " << next;
      }
      return ss.str();
    }
  };

  struct Bin {
    size_t bin_size = 0;

    class ChunkComparator {
    public:
      explicit ChunkComparator(BFCAllocator *allocator)
          : _allocator(allocator) {}
      bool operator()(const ChunkHandle ha, const ChunkHandle hb) const {
        const Chunk *a = _allocator->ChunkFromHandle(ha);
        const Chunk *b = _allocator->ChunkFromHandle(hb);
        if (a->size != b->size) {
          return a->size < b->size;
        }
        return a->ptr < b->ptr;
      }

    private:
      BFCAllocator *_allocator; // not owned
    };

    typedef std::set<ChunkHandle, ChunkComparator> FreeChunkSet;

    FreeChunkSet free_chunks;
    Bin(BFCAllocator *allocator, size_t bs)
        : bin_size(bs), free_chunks(ChunkComparator(allocator)) {}
  };

  class AllocationRegion {
  public:
    AllocationRegion(void *ptr, size_t memory_size)
        : ptr_(ptr), memory_size_(memory_size),
          end_ptr_(
              static_cast<void *>(static_cast<char *>(ptr_) + memory_size_)) {
      DCHECK_EQ(size_t{0}, memory_size % MIN_ALLOCATION_SIZE);
      const size_t n_handles =
          (memory_size + MIN_ALLOCATION_SIZE - 1) / MIN_ALLOCATION_SIZE;
      handles_.resize(n_handles, MIN_ALLOCATION_SIZE);
    }

    AllocationRegion() = default;
    AllocationRegion(AllocationRegion &&other) { Swap(&other); }
    AllocationRegion &operator=(AllocationRegion &&other) {
      Swap(&other);
      return *this;
    }

    void *ptr() const { return ptr_; }
    void *end_ptr() const { return end_ptr_; }
    size_t memory_size() const { return memory_size_; }
    void extend(size_t size) {
      memory_size_ += size;
      DCHECK_EQ(size_t{0}, memory_size_ % MIN_ALLOCATION_SIZE);

      end_ptr_ = static_cast<void *>(static_cast<char *>(end_ptr_) + size);
      const size_t n_handles =
          (memory_size_ + MIN_ALLOCATION_SIZE - 1) / MIN_ALLOCATION_SIZE;
      handles_.resize(n_handles, INVALID_CHUNK_HANDLE);
    }
    ChunkHandle get_handle(const void *p) const {
      return handles_[IndexFor(p)];
    }
    void set_handle(const void *p, ChunkHandle h) { handles_[IndexFor(p)] = h; }
    void erase(const void *p) { set_handle(p, INVALID_CHUNK_HANDLE); }

  private:
    void Swap(AllocationRegion *other) {
      std::swap(ptr_, other->ptr_);
      std::swap(memory_size_, other->memory_size_);
      std::swap(end_ptr_, other->end_ptr_);
      std::swap(handles_, other->handles_);
    }

    size_t IndexFor(const void *p) const {
      std::uintptr_t p_int = reinterpret_cast<std::uintptr_t>(p);
      std::uintptr_t base_int = reinterpret_cast<std::uintptr_t>(ptr_);
      DCHECK_GE(p_int, base_int);
      DCHECK_LT(p_int, base_int + memory_size_);
      return static_cast<size_t>(((p_int - base_int) >> MIN_ALLOCATION_BITS));
    }

    // Metadata about the allocation region.
    void *ptr_ = nullptr;
    size_t memory_size_ = 0;
    void *end_ptr_ = nullptr;

    // Array of size "memory_size / kMinAllocationSize".  It is
    // indexed by (p-base) / kMinAllocationSize, contains ChunkHandle
    // for the memory allocation represented by "p"
    std::vector<ChunkHandle> handles_;

    CHIME_DISALLOW_COPY_AND_ASSIGN(AllocationRegion);
  };

  class RegionManager {
  public:
    RegionManager() {}
    ~RegionManager() {}

    void AddAllocationRegion(void *ptr, size_t memory_size) {
      // Insert sorted by end_ptr.
      auto entry =
          std::upper_bound(regions_.begin(), regions_.end(), ptr, &Comparator);
      regions_.insert(entry, AllocationRegion(ptr, memory_size));
    }

    // Adds an alloation region for the given ptr and size, potentially
    // extending a region if ptr matches the end_ptr of an existing region.
    // If a region is extended, returns a pointer to the extended region so that
    // the BFC allocator can reason about chunkification.
    AllocationRegion *AddOrExtendAllocationRegion(void *ptr,
                                                  size_t memory_size) {
      // Insert sorted by end_ptr.
      auto entry =
          std::upper_bound(regions_.begin(), regions_.end(), ptr, &Comparator);
      // Check if can be coalesced with preceding region.
      if (entry != regions_.begin()) {
        auto preceding_region = entry - 1;
        if (preceding_region->end_ptr() == ptr) {

          preceding_region->extend(memory_size);
          return &*preceding_region;
        }
      }
      regions_.insert(entry, AllocationRegion(ptr, memory_size));
      return nullptr;
    }

    std::vector<AllocationRegion>::iterator
    RemoveAllocationRegion(std::vector<AllocationRegion>::iterator it) {
      return regions_.erase(it);
    }

    ChunkHandle get_handle(const void *p) const {
      return RegionFor(p)->get_handle(p);
    }

    void set_handle(const void *p, ChunkHandle h) {
      return MutableRegionFor(p)->set_handle(p, h);
    }
    void erase(const void *p) { return MutableRegionFor(p)->erase(p); }

    const std::vector<AllocationRegion> &regions() const { return regions_; }

  private:
    static bool Comparator(const void *ptr, const AllocationRegion &other) {
      return ptr < other.end_ptr();
    }

    AllocationRegion *MutableRegionFor(const void *p) {
      return const_cast<AllocationRegion *>(RegionFor(p));
    }

    const AllocationRegion *RegionFor(const void *p) const {
      auto entry =
          std::upper_bound(regions_.begin(), regions_.end(), p, &Comparator);

      if (entry != regions_.end()) {
        return &(*entry);
      }

      LOG(FATAL) << "Could not find Region for " << p;
      return nullptr;
    }

  private:
    std::vector<AllocationRegion> regions_;
  };

  void *AllocateInternal(size_t unused_align, size_t num_bytes);

  void *FindPtrFromChunks(BinNum bin_num, size_t rounded_bytes,
                          size_t req_size);

  Chunk *ChunkFromHandle(ChunkHandle h);

  size_t RoundedBytes(size_t bytes) const;

  /// Returns floor(log2(n))
  inline int Log2FloorNonZero(uint64_t n) const {
#ifdef __GNUC__
    return 63 ^ __builtin_clzll(n);
#else
    return Log2FloorNonZeroSlow(n);
#endif
  }

  inline int Log2FloorNonZeroSlow(uint64_t n) const {
    int r = 0;
    while (n > 0) {
      r++;
      n >>= 1;
    }
    return r - 1;
  }

  BinNum BinNumFromSize(size_t size) const {
    // log2(size / 256)
    uint64_t v = std::max<size_t>(size, 256) >> MIN_ALLOCATION_BITS;
    int b = std::min(NUM_BINS - 1, Log2FloorNonZero(v));
    return b;
  }

  size_t BinNumToSize(BinNum bin_num) const {
    return static_cast<size_t>(256) << bin_num;
  }

  Bin *BinFromIndex(BinNum index) {
    return reinterpret_cast<Bin *>(&(bins_space_[index * sizeof(Bin)]));
  }

  void RemoveFreeChunkIterFromBin(Bin::FreeChunkSet *free_chunks,
                                  Bin::FreeChunkSet::iterator iter);

  void RemoveFreeChunkFromBin(ChunkHandle h);

  void SplitChunk(ChunkHandle h, size_t rounded_bytes);

  ChunkHandle AllocateChunk();

  void InsertFreeChunkIntoBin(ChunkHandle h);

  void DeallocateChunk(ChunkHandle h);

  bool Extend(size_t alignment, size_t rounded_bytes);

  ChunkHandle TryToCoalesce(ChunkHandle h);

  void Merge(ChunkHandle h1, ChunkHandle h2);

  void DeleteChunk(ChunkHandle h);

  void DeallocateInternal(void *ptr);

  void MarkFree(ChunkHandle h);

  std::mutex mutex_;

  Allocator *allocator_; // not owned
  size_t memory_limit_;

  int64_t next_allocation_id_;

  ChunkHandle free_chunks_;

  std::vector<Chunk> chunks_;
  char bins_space_[sizeof(Bin) * NUM_BINS];

  size_t total_bytes_;
  size_t curr_allocation_bytes_;

  RegionManager region_manager_;
};

} // namespace memory

#endif // BFC_ALLOCATOR_BFC_ALLOCATOR_H_
