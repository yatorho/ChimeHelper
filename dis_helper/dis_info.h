#ifndef DIS_HELPER_DIS_INFO_H_
#define DIS_HELPER_DIS_INFO_H_

#include <string>
#include <map>

namespace dis {

namespace core {

struct Address {
  int port;
  std::string ip;

  bool operator==(const Address &other) const {
    return port == other.port && ip == other.ip;
  }
};


typedef std::map<int, Address> AddressMap;

AddressMap &GetGlobalAddressMap();

} // namespace core
} // namespace dis

#endif // DIS_HELPER_DIS_INFO_H_
