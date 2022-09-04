#include <iostream>
#include <thread>
#include <atomic>


int main() {
  std::atomic_int a{0};

  std::thread t1([&a]() {
    for (int i = 0; i < 10000; i++) {
      a++;
    }
  });

  std::thread t2([&a]() {
    for (int i = 0; i < 10000; i++) {
      a++;
    }
  });

  t1.join();
  t2.join();

  std::cout << a << std::endl;
}
