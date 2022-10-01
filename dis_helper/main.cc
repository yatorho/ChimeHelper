#include "dis_helper/dis_info.h"
#include "dis_helper/graph.h"
#include "dis_helper/matrix.h"
#include <iostream>

#include "dis_helper/master.h"
#include "dis_helper/worker_loop.h"


void MasterDo(dis::DisServiceClient *client) {

  dis::core::Matrix *matrixa = new dis::core::Matrix(6, 5, "a");
  dis::core::Matrix *matrixb = new dis::core::Matrix(5, 8, "b");
  dis::core::Matrix *matrixd = new dis::core::Matrix(8, 6, "d");

  for (int i = 0; i < 40; i++) {
    if (i < 30) {
      matrixa->data.get()[i] = i;
    }
    matrixb->data.get()[i] = i;
  }

  for (int i = 0; i < 48; i++) {
    matrixd->data.get()[i] = i;
  }

  matrixa->computed = true;
  matrixb->computed = true;
  matrixd->computed = true;

  dis::core::Address master_address = dis::core::GetGlobalAddressMap()[0];
  dis::core::Address worker_address = dis::core::GetGlobalAddressMap()[1];

  dis::core::Matrix *matrix1 = nullptr;
  dis::core::Matrix *matrix2 = nullptr;

  matrix1 = client->GetMatrixFromRemote(master_address, "a", "m1", 0, 3, 0, 5);
  matrix2 = client->GetMatrixFromRemote(master_address, "b", "m2", 0, 5, 0, 8);

  dis::core::Matrix *partial_res0 =
      dis::core::MatrixMatMul(matrix1, matrix2, "res0");

  dis::core::Matrix *partial_res1 = nullptr;
  while (partial_res1 == nullptr)
    partial_res1 =
        client->GetMatrixFromRemote(worker_address, "res1", "res1", 0, 3, 0, 8);

  dis::core::Matrix *res = dis::core::MergeMatrixVertically(
      partial_res0, partial_res1, "final_res0");

  // Print result
  dis::core::ShowMatrixValue(res);

  dis::core::Matrix *matrix3 = nullptr;
  dis::core::Matrix *matrix4 = nullptr;

  while (matrix3 == nullptr || matrix4 == nullptr) {
    matrix3 =
        client->GetMatrixFromRemote(master_address, "d", "m3", 0, 8, 0, 6);
    matrix4 = client->GetMatrixFromRemote(master_address, "final_res0", "m4", 0,
                                          6, 0, 4);
  }

  dis::core::Matrix *partial_res2 =
      dis::core::MatrixMatMul(matrix3, matrix4, "res2");

  dis::core::Matrix *partial_res3 = nullptr;
  while (partial_res3 == nullptr) {
    partial_res3 =
        client->GetMatrixFromRemote(worker_address, "res3", "res3", 0, 8, 0, 4);
  }

  dis::core::Matrix *final_res =
      dis::core::MergeMatrixParallely(partial_res2, partial_res3, "final_res");
  dis::core::ShowMatrixValue(final_res);
  float sum = dis::core::SumMatrix(final_res);
  std::cout << sum << std::endl;
}

void WorkerDo(dis::DisServiceClient *client) {

  ::dis::core::Address master_address{
      std::stoi(std::string(getenv("MASTER_PORT"))),
      std::string(getenv("MASTER_ADDRESS"))};

  dis::core::Matrix *m1 = nullptr;
  dis::core::Matrix *m2 = nullptr;
  while (m1 == nullptr || m2 == nullptr) {
    m1 = client->GetMatrixFromRemote(master_address, "a", "m1", 3, 6, 0, 5);
    m2 = client->GetMatrixFromRemote(master_address, "b", "m2", 0, 5, 0, 8);
  }

  LOG(INFO) << "Got remote matrix a and b";
 
  dis::core::MatrixMatMul(m1, m2, "res1");

  dis::core::Matrix *m3 = nullptr;
  dis::core::Matrix *m4 = nullptr;

  while (m3 == nullptr || m4 == nullptr) {
    m3 = client->GetMatrixFromRemote(master_address, "d", "m3", 0, 8, 0, 6);
    m4 = client->GetMatrixFromRemote(master_address, "final_res0", "m4", 0, 6,
                                     4, 8);
  }

  dis::core::MatrixMatMul(m3, m4, "res3");

  // Wait for master to get the matrix res3
  while(1);

}

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
  dis::DisServiceClient *client = dis::SynceEnv();
  if ((m_address + m_port) == (l_address + l_port)) {
    MasterDo(client);
  } else {
    WorkerDo(client);
  }

  // Release server resource
  dis::FreeDisSerivce(env);
  delete client;
}
