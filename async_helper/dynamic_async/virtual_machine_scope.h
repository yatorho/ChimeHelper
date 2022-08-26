#ifndef ASYNC_HELPER_DYNAMIC_ASYNC_VIRTUAL_MACTHINE_SCOPE_H_
#define ASYNC_HELPER_DYNAMIC_ASYNC_VIRTUAL_MACTHINE_SCOPE_H_

#include "virtual_machine.h"

namespace async_helper {

/// This Scope should be included in the scope of the main thread.
struct VirtualMachineScope {
  VirtualMachineScope() { _vm = VirtualMachine::Singleton(); }
  ~VirtualMachineScope() { _vm->Close(); }

  VirtualMachine *_vm;
};

} // namespace async_helper

#endif // ASYNC_HELPER_DYNAMIC_ASYNC_VIRTUAL_MACTHINE_SCOPE_H_
