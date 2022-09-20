#include "dis_helper/dis_info.h"

namespace dis {
namespace core {

AddressMap address_map; // Make sure thread safe

AddressMap GetGlobalAddressMap() { return address_map; }

void SetGlobalAddressMap(const AddressMap &address) { address_map = address; }

void InsertGlobalAddressMap(int rank, const Address &address) {
  address_map[rank] = address;
}

} // namespace core
} // namespace dis