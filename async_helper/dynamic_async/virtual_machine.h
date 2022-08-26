#ifndef ASYNC_HELPER_VISUAL_MACHINE_H_
#define ASYNC_HELPER_VISUAL_MACHINE_H_

#include "async_helper/static_sync/op.h"
#include "chime/core/platform/cpu_info.h"
#include "chime/core/platform/threadpool.h"
#include "virtual_machine_engine.h"

#include "async_helper/static_sync/graph.h"

#include <mutex>
#include <queue>
#include <thread>

namespace async_helper {

class VirtualMachine {
public:
  static VirtualMachine *Singleton();

  // private:
  ~VirtualMachine() { LOG(FATAL) << "VirtualMachine should not be destroyed"; }

  VirtualMachine() : _close(false) {
    _schedule_thread = chime::platform::Env::Default()->StartThread(
        chime::platform::ThreadOptions(), "schedule pool",
        [this]() { ScheduleLoop(); });
  }

  void ScheduleLoop() {
    while (!_close) {
      Graph *graph = GetDefaultGraph();
      CHECK(graph != nullptr) << "ScheduleLoop: nullptr graph";

      const std::vector<std::pair<int, Operator *>> &op_ps =
          graph->OperatorsWithPriority();

      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      for (auto &op_p : op_ps) {
        if (std::find(_done_ops.begin(), _done_ops.end(), op_p.second) !=
            _done_ops.end())
          continue;
        if (std::find(_scheduling_ops.begin(), _scheduling_ops.end(),
                      op_p.second) != _scheduling_ops.end()) {
          if (op_p.second->_computed) {
            _done_ops.push_back(op_p.second);
            _scheduling_ops.erase(std::find(
                _scheduling_ops.begin(), _scheduling_ops.end(), op_p.second));
          }

          continue;
        }

        if (op_p.second->IsReadyToBeScheduled()) {
          Operator *op = op_p.second;
          LOG(INFO) << "VM schedule: " << op->Name();
          _engine.Recieve([op]() { op->Compute(); });
          _scheduling_ops.push_back(op);
        }
      }
    }
    LOG(INFO) << "ScheduleLoop: exit";
  }

  void Close() {
    _close = true;
    delete _schedule_thread;
    _engine.Synce();
    for (auto op : _scheduling_ops) {
      _done_ops.push_back(op);
    }
    _scheduling_ops.clear();
  }

  std::vector<Operator *> _done_ops;
  std::vector<Operator *> _scheduling_ops;
  bool _close;
  VirtualMachineEngine _engine;
  chime::platform::Thread *_schedule_thread;
};

} // namespace async_helper

#endif // ASYNC_HELPER_VISUAL_MACHINE_H_
