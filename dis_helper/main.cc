#include <iostream>

#include "dis_helper/worker_loop.h"
#include "dis_helper/master.h"

int main() { 
  // Get environment variable
  char *m_port = getenv("MASTER_PORT");
  char *m_address = getenv("MASTER_ADDRESS");

  char *l_port = getenv("LOCAL_PORT");
  char *l_address = getenv("LOCAL_ADDRESS");

  if (m_port == NULL || m_address == NULL || l_port == NULL || l_address == NULL) {
    std::cout << "Environment variables not set" << std::endl;
    return 1;
  }

  // Start Server and Client
  auto env = dis::EnvStart();
  dis::SynceEnv();

  // Release server resource
  dis::FreeDisSerivce(env);
}
