#include <half.hpp>
#include <iostream>
#include <chrono>

using half_float::half;

int main() {
  const int N = 10000;
  half *half_add1 = (half *) malloc(N * sizeof(half));
  half *half_add2 = (half *) malloc(N * sizeof(half));
  half *half_sum = (half *) malloc(N * sizeof(half));
  
  float *float_add1 = (float *) malloc(N * sizeof(float));
  float *float_add2 = (float *) malloc(N * sizeof(float));
  float *float_sum = (float *) malloc(N * sizeof(float));

  for (int i = 0; i < N; i++) {
    half_add1[i] = half(i);
    half_add2[i] = half(i);
    float_add1[i] = i;
    float_add2[i] = i;
  }

  auto start_time_half = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; i++) {
    half_sum[i] = half_add1[i] + half_add2[i];
  }
  auto end_time_half = std::chrono::high_resolution_clock::now();

  auto start_time_float = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; i++) {
    float_sum[i] = float_add1[i] + float_add2[i];
  }
  auto end_time_float = std::chrono::high_resolution_clock::now();

  std::cout << "half: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time_half - start_time_half).count() << "us" << std::endl;
  std::cout << "float: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time_float - start_time_float).count() << "us" << std::endl;
  
  return 0;
}