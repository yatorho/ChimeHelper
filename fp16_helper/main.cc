#include <chrono>
#include <cstdint>
#include <fp16.h>
#include <iostream>
#include <new>

int main() {
  const int N = 10000;

  uint16_t *halfs_add1 =
      static_cast<uint16_t *>(::operator new(sizeof(uint16_t) * N));
  uint16_t *halfs_add2 =
      static_cast<uint16_t *>(::operator new(sizeof(uint16_t) * N));
  uint16_t *halfs_sum =
      static_cast<uint16_t *>(::operator new(sizeof(uint16_t) * N));

  float *floats_add1 = static_cast<float *>(::operator new(sizeof(float) * N));
  float *floats_add2 = static_cast<float *>(::operator new(sizeof(float) * N));
  float *floats_sum = static_cast<float *>(::operator new(sizeof(float) * N));

  for (int i = 0; i < N; i++) {
    halfs_add1[i] = fp16_ieee_from_fp32_value(i);
    halfs_add2[i] = fp16_ieee_from_fp32_value(i);
    floats_add1[i] = i;
    floats_add2[i] = i;
  }

  auto start_time_half = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; i++) {
    halfs_sum[i] =
        fp16_ieee_from_fp32_value(fp16_ieee_to_fp32_value(halfs_add1[i]) +
                                  fp16_ieee_to_fp32_value(halfs_add2[i]));
  }
  auto end_time_half = std::chrono::high_resolution_clock::now();

  auto start_time_float = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; i++) {
    floats_sum[i] = floats_add1[i] + floats_add2[i];
  }
  auto end_time_float = std::chrono::high_resolution_clock::now();

  std::cout << "half: "
            << std::chrono::duration_cast<std::chrono::microseconds>(
                   end_time_half - start_time_half)
                   .count()
            << "us" << std::endl;
  std::cout << "float: "
            << std::chrono::duration_cast<std::chrono::microseconds>(
                   end_time_float - start_time_float)
                   .count()
            << "us" << std::endl;

  ::operator delete(halfs_add1);
  ::operator delete(halfs_add2);
  ::operator delete(halfs_sum);

  ::operator delete(floats_add1);
  ::operator delete(floats_add2);
  ::operator delete(floats_sum);
}