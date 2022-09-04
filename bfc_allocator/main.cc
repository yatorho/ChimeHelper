// #include "bfc_allocator/bfc_allocator.h"

#include <vector>
#include <iostream>

int main() {
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.resize(4, 5);

  for (auto i : v) {
    std::cout << i << std::endl;
  }
}