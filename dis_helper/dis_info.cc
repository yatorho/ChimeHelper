#include "dis_helper/dis_info.h"
#include "dis_helper/env.pb.h"
#include <vector>

namespace dis {
namespace core {

AddressMap address_map; // Make sure thread safe

AddressMap& GetGlobalAddressMap() { return address_map; }

} // namespace core
} // namespace dis
