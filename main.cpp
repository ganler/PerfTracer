#include <future>
#include <iostream>

#include "PerfTracer.hpp"

void lrznb() {
  using namespace std::chrono_literals;
  thread_local int i = 0;
  THREAD_PERF_TRACE(__FUNCTION__+std::to_string(++i))
  std::this_thread::sleep_for(50ms);
}

int main()
{
  {
    PERF_TRACE(__FUNCTION__);
    {
      auto foo = std::async([](){
        lrznb();
        lrznb();
      });
      lrznb();
      lrznb();
      lrznb();
    }
  }
}

