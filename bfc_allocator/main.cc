#include "bfc_allocator/bfc_allocator.h"

#include <iostream>
#include <vector>

using memory::BFCAllocator;
using memory::CPUAllocator;

int main() {
  CPUAllocator cpu_allocator;
  BFCAllocator bfc_allocator(&cpu_allocator, 1024, true);

  void *ptr = bfc_allocator.Allocate(513, 8);
  // Check ptr is aligned to at least 256 bytes.
  if (reinterpret_cast<uintptr_t>(ptr) % 256 != 0) {
    std::cout << "ptr is not aligned to 256 bytes" << std::endl;
  } else {
    std::cout << "ptr is aligned to 256 bytes" << std::endl;
  }
  bfc_allocator.Deallocate(ptr);
}