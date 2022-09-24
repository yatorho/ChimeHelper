#include "dis_helper/graph.h"
#include "dis_helper/matrix.h"
#include <iostream>

#include "dis_helper/master.h"
#include "dis_helper/worker_loop.h"

int main() {
  // Get environment variable
  std::string m_port = getenv("MASTER_PORT");
  std::string m_address = getenv("MASTER_ADDRESS");

  std::string l_port = getenv("LOCAL_PORT");
  std::string l_address = getenv("LOCAL_ADDRESS");

  if (m_port.empty() || m_address.empty() || l_port.empty() ||
      l_address.empty()) {
    std::cout << "Environment variables not set" << std::endl;
    return 1;
  }

  // Start Server and Client
  auto env = dis::EnvStart();
  dis::SynceEnv();

  if ((m_address + m_port) == (l_address + l_port)) {

    dis::core::Matrix *a_matrix = new dis::core::Matrix(6, 5,"a");
    dis::core::Matrix *b_matrix = new dis::core::Matrix(5, 8,"b");

    for (int i = 0; i < 40; i++) {
      if (i < 30) {
        a_matrix->data.get()[i] = i;
      }
      b_matrix->data.get()[i] = i;
    }
  }

  // Release server resource
  dis::FreeDisSerivce(env);
}
