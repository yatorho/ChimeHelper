#include "virtual_machine.h"

namespace async_helper {

VirtualMachine *VirtualMachine::Singleton() {
  static VirtualMachine *vm = new VirtualMachine;
  return vm;
}

}
