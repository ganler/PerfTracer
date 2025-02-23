#include <future>
#include <iostream>

#include "perf_trace.hpp"

void lrznb() {
    using namespace std::chrono_literals;
    thread_local int i = 0;
    THREAD_PERF_TRACE(__FUNCTION__+std::to_string(++i))
    std::this_thread::sleep_for(50ms);
}

void foo() {
    using namespace std::chrono_literals;
    PERF_TRACE(__FUNCTION__)
    std::this_thread::sleep_for(50ms);
}

int main()
{

    PERF_TRACE(__FUNCTION__);
    {
        {
            PERF_TRACE("father of foo")
        }
        {
            PERF_TRACE("father of bar")
        }
        {
            PERF_TRACE("father of jar")
        }
        for (int i = 0; i < 4; ++i) {
            foo();
        }
    }

}

