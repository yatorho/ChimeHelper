#include "async_helper/virtual_machine.h"

namespace async_helper {

VirtualMachine *VirtualMachine::Singleton() {
  static VirtualMachine vm;
  return &vm;
}

}
