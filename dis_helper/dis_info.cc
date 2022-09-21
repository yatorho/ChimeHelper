#include "dis_helper/dis_info.h"

namespace dis {
namespace core {

AddressMap address_map; // Make sure thread safe

AddressMap& GetGlobalAddressMap() { return address_map; }

} // namespace core
} // namespace dis