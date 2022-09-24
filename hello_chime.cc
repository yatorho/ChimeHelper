#include "chime/core/platform/threadpool.h"
#include <iostream>

using chime::platform::Env;
using chime::platform::ThreadPool;

int main() {
  ThreadPool pool(Env::Default(), "hello", 1);
  pool.Schedule([]() { std::cout << "Hello Chime!" << std::endl; });
  pool.Wait();
}
