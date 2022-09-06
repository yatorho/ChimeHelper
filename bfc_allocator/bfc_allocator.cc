
#include "bfc_allocator/bfc_allocator.h"

#include "chime/core/platform/logging.hpp"

namespace memory {

constexpr BFCAllocator::ChunkHandle BFCAllocator::INVALID_CHUNK_HANDLE;

BFCAllocator::BFCAllocator(Allocator *allocator, size_t memory_limit,
                           bool allow_growth)
    : allocator_(allocator), memory_limit_(memory_limit),
      next_allocation_id_(1), free_chunks_(INVALID_CHUNK_HANDLE),
      total_bytes_(0) {
  for (BinNum b = 0; b < NUM_BINS; ++b) {
    size_t bin_size = BinNumToSize(b);

    new (BinFromIndex(b)) Bin(this, bin_size);
  }
  if (allow_growth) {
    curr_allocation_bytes_ =
        RoundedBytes(std::min(memory_limit, size_t{2 << 20})); // 128MB
  } else {
    curr_allocation_bytes_ = RoundedBytes(memory_limit);
  }
}

BFCAllocator::~BFCAllocator() {
  // Cause we don't implement a SubAllocator here, we just free all the memory
  // by traversing the chunks_.  This may cause some memory leak if there are
  // some AllocationRegion has been extended.
  // TODO: Implement a SubAllocator to manage the memory. Like following:
  /*
   * for (const auto &region : region_manager_.regions()) {
   *   allocator_->Deallocate(region.ptr(), region.memory_size());
   * }
   */

  for (auto &region : region_manager_.regions()) {
    allocator_->Deallocate(region.ptr());
  }

  for (BinNum b = 0; b < NUM_BINS; b++) {
    BinFromIndex(b)->~Bin();
  }
}

BFCAllocator::Chunk *BFCAllocator::ChunkFromHandle(ChunkHandle h) {
  return &chunks_[h];
}

size_t BFCAllocator::RoundedBytes(size_t bytes) const {
  return ((bytes + MIN_ALLOCATION_SIZE - 1) / MIN_ALLOCATION_SIZE) *
         MIN_ALLOCATION_SIZE;
}

void *BFCAllocator::FindPtrFromChunks(BinNum bin_num, size_t rounded_bytes,
                                      size_t req_size) {
  for (; bin_num < NUM_BINS; bin_num++) {
    Bin *b = BinFromIndex(bin_num);

    for (auto iter = b->free_chunks.begin(); iter != b->free_chunks.end();
         iter++) {
      const ChunkHandle h = *iter;
      Chunk *c = ChunkFromHandle(h);

      CHECK(!c->IsUse());
      if (c->size >= rounded_bytes) {
        RemoveFreeChunkIterFromBin(&b->free_chunks, iter);

        const int64_t max_internal_fragmentation_bytes = 128 << 20; // 128MB

        if (c->size >= rounded_bytes * 2 ||
            static_cast<int64_t>(c->size) - rounded_bytes >=
                max_internal_fragmentation_bytes) {
          SplitChunk(h, rounded_bytes);
          c = ChunkFromHandle(h);
        }

        c->req_size = req_size;
        c->allocation_id = next_allocation_id_;
        next_allocation_id_++;

        return c->ptr;
      }
    }
  }
  return nullptr;
}

BFCAllocator::ChunkHandle BFCAllocator::AllocateChunk() {
  if (free_chunks_ != INVALID_CHUNK_HANDLE) {
    ChunkHandle h = free_chunks_;
    Chunk *c = ChunkFromHandle(h);
    free_chunks_ = c->next;
    return h;
  } else {
    ChunkHandle h = chunks_.size();
    chunks_.resize(h + 1);
    return h;
  }
}

void BFCAllocator::DeallocateChunk(ChunkHandle h) {
  Chunk *c = ChunkFromHandle(h);
  c->allocation_id = -1;
  c->bin_num = INVALID_BIN_NUM;
  c->next = free_chunks_;
  free_chunks_ = h;
}

void BFCAllocator::SplitChunk(ChunkHandle h, size_t rounded_bytes) {
  ChunkHandle h_new_chunk = AllocateChunk();

  Chunk *c = ChunkFromHandle(h);
  DCHECK(!c->IsUse() && (c->bin_num == INVALID_BIN_NUM));

  Chunk *new_chunk = ChunkFromHandle(h_new_chunk);
  new_chunk->ptr =
      static_cast<void *>((static_cast<char *>(c->ptr) + rounded_bytes));

  DCHECK_EQ(region_manager_.get_handle(new_chunk->ptr), INVALID_CHUNK_HANDLE);
  region_manager_.set_handle(new_chunk->ptr, h_new_chunk);

  new_chunk->size = c->size - rounded_bytes;
  c->size = rounded_bytes;

  new_chunk->allocation_id = -1;

  // Maintain the pointers.
  // c <-> c_neighbor becomes
  // c <-> new_chunk <-> c_neighbor
  ChunkHandle h_neighbor = c->next;
  new_chunk->prev = h;
  new_chunk->next = h_neighbor;
  c->next = h_new_chunk;
  if (h_neighbor != INVALID_CHUNK_HANDLE) {
    Chunk *c_neighbor = ChunkFromHandle(h_neighbor);
    c_neighbor->prev = h_new_chunk;
  }

  InsertFreeChunkIntoBin(h_new_chunk);
}

void *BFCAllocator::AllocateInternal(size_t unused_align,
                                     size_t num_bytes) { // 512 + 256
  if (num_bytes == 0) {
    LOG(INFO) << "Tried to allocate 0 bytes";
    return nullptr;
  }
  size_t rounded_bytes = RoundedBytes(num_bytes);

  BinNum bin_num = BinNumFromSize(rounded_bytes);

  std::lock_guard<std::mutex> lock(mutex_);
  void *ptr = FindPtrFromChunks(bin_num, rounded_bytes, num_bytes);
  if (ptr != nullptr) {
    LOG(INFO) << "AllocateInternal: " << ptr << " " << num_bytes;
    return ptr;
  }

  // Try to extend the memory.
  if (Extend(unused_align, rounded_bytes)) {
    ptr = FindPtrFromChunks(bin_num, rounded_bytes, num_bytes);
    if (ptr != nullptr) {
      return ptr;
    }
  }

  // Failed to allocate memory.  We need to run out of memory error.
  LOG(WARNING) << "Failed to allocate memory";
  return nullptr;
}

bool BFCAllocator::Extend(size_t alignment, size_t rounded_bytes) {
  size_t available_bytes = memory_limit_ - total_bytes_;

  available_bytes =
      (available_bytes / MIN_ALLOCATION_SIZE) * MIN_ALLOCATION_SIZE;

  if (available_bytes < rounded_bytes) {
    return false;
  }

  bool increased_allocation = false;
  while (rounded_bytes > curr_allocation_bytes_) {
    curr_allocation_bytes_ *= 2;
    increased_allocation = true;
  }

  // Trying to allocate
  size_t bytes = std::min(curr_allocation_bytes_, available_bytes);
  void *mem_addr = allocator_->Allocate(bytes, alignment);

  if (mem_addr == nullptr) {
    static constexpr float BACH_PEDAL_FACTOR = 0.9f;

    while (mem_addr == nullptr) {
      bytes = RoundedBytes(bytes * BACH_PEDAL_FACTOR);
      if (bytes < rounded_bytes)
        break;
      mem_addr = allocator_->Allocate(bytes, alignment);
    }
  }

  if (mem_addr == nullptr) {
    return false;
  }

  if (!increased_allocation) {
    curr_allocation_bytes_ *= 2;
  }

  total_bytes_ += bytes;

  AllocationRegion *maybe_extended_region = nullptr;
  maybe_extended_region =
      region_manager_.AddOrExtendAllocationRegion(mem_addr, bytes);

  ChunkHandle h = AllocateChunk();
  Chunk *c = ChunkFromHandle(h);
  c->ptr = mem_addr;
  c->size = bytes;
  c->allocation_id = -1;
  c->prev = INVALID_CHUNK_HANDLE;
  c->next = INVALID_CHUNK_HANDLE;

  DCHECK_EQ(region_manager_.get_handle(c->ptr), INVALID_CHUNK_HANDLE);
  region_manager_.set_handle(c->ptr, h);

  // If the region was extended, then there exists a previous chunk that should
  // be linked to the new chunk.
  if (maybe_extended_region != nullptr) {
    ChunkHandle prev =
        maybe_extended_region->get_handle(maybe_extended_region->ptr());
    Chunk *prev_chunk = ChunkFromHandle(prev);
    while (prev_chunk->next != INVALID_CHUNK_HANDLE) {
      prev = prev_chunk->next;
      prev_chunk = ChunkFromHandle(prev);
    }
    c->prev = prev;
    prev_chunk->next = h;
  }
  InsertFreeChunkIntoBin(TryToCoalesce(h));
  return true;
}

BFCAllocator::ChunkHandle BFCAllocator::TryToCoalesce(ChunkHandle h) {
  Chunk *c = ChunkFromHandle(h);

  ChunkHandle coalesced = h;

  // If the next chunk is free, merge it into c and delete it.
  if (c->next != INVALID_CHUNK_HANDLE && !ChunkFromHandle(c->next)->IsUse()) {
    RemoveFreeChunkFromBin(c->next);
    Merge(h, c->next);
  }

  // If the previous chunk is free, merge c into it and delete c.
  if (c->prev != INVALID_CHUNK_HANDLE && !ChunkFromHandle(c->prev)->IsUse()) {
    coalesced = c->prev;
    RemoveFreeChunkFromBin(c->prev);
    Merge(c->prev, h);
  }

  return coalesced;
}

void BFCAllocator::Merge(ChunkHandle h1, ChunkHandle h2) {
  Chunk *c1 = ChunkFromHandle(h1);
  Chunk *c2 = ChunkFromHandle(h2);

  // Only merge if the two chunks are adjacent.
  CHECK(c1->next == h2 && c2->prev == h1);

  // We can only merge chunks that are not in use.
  CHECK(!c1->IsUse() && !c2->IsUse());

  // Fix up neighbor pointers
  //
  // c1 <-> c2 <-> c3 should become
  // c1 <-> c3
  ChunkHandle h3 = c2->next;
  c1->next = h3;
  if (h3 != INVALID_CHUNK_HANDLE) {
    Chunk *c3 = ChunkFromHandle(h3);
    c3->prev = h1;
  }

  c1->size += c2->size;
  DeleteChunk(h2);
}

void BFCAllocator::DeleteChunk(ChunkHandle h) {
  Chunk *c = ChunkFromHandle(h);
  region_manager_.erase(c->ptr);
  DeallocateChunk(h);
}

void BFCAllocator::InsertFreeChunkIntoBin(ChunkHandle h) {
  Chunk *c = ChunkFromHandle(h);
  CHECK(!c->IsUse() && (c->bin_num == INVALID_BIN_NUM))
      << "is use: " << c->IsUse() << " bin_num: " << c->bin_num;
  BinNum bin_num = BinNumFromSize(c->size);
  Bin *bin = BinFromIndex(bin_num);
  c->bin_num = bin_num;
  bin->free_chunks.insert(h);
}

void BFCAllocator::RemoveFreeChunkIterFromBin(
    Bin::FreeChunkSet *free_chunks, Bin::FreeChunkSet::iterator iter) {
  ChunkHandle h = *iter;
  Chunk *c = ChunkFromHandle(h);
  CHECK(!c->IsUse() && (c->bin_num != INVALID_BIN_NUM));
  free_chunks->erase(iter);
  c->bin_num = INVALID_BIN_NUM;
}

void BFCAllocator::RemoveFreeChunkFromBin(ChunkHandle h) {
  Chunk *c = ChunkFromHandle(h);
  CHECK(!c->IsUse() && (c->bin_num != INVALID_BIN_NUM));
  CHECK_GT(BinFromIndex(c->bin_num)->free_chunks.erase(h), 0ull)
      << "Could not find chunk in bin";
  c->bin_num = INVALID_BIN_NUM;
}

void *BFCAllocator::Allocate(size_t size, size_t align) {
  size_t rounded_bytes = RoundedBytes(size);
  size_t no_use_alignment = MIN_ALLOCATION_SIZE;

  void *res = AllocateInternal(no_use_alignment, rounded_bytes);
  if (res == nullptr) {
    LOG(WARNING) << "Out of memory when allocating " << size << " bytes";
    return res;
  }
  return res;
}

void BFCAllocator::MarkFree(ChunkHandle h) {
  Chunk *c = ChunkFromHandle(h);
  CHECK(c->IsUse() && (c->bin_num == INVALID_BIN_NUM));
  c->allocation_id = -1;
}

void BFCAllocator::DeallocateInternal(void *ptr) {
  std::lock_guard<std::mutex> lock(mutex_);

  ChunkHandle h = region_manager_.get_handle(ptr);
  CHECK(h != INVALID_CHUNK_HANDLE) << "Invalid pointer: " << ptr;

  MarkFree(h);

  InsertFreeChunkIntoBin(TryToCoalesce(h));
}

void BFCAllocator::Deallocate(void *ptr) { DeallocateInternal(ptr); }

} // namespace memory
