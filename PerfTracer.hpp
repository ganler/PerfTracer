//
// Created by ganler on 3/11/20.
//

#pragma once

#include <cassert>
#include <chrono>
#include <iomanip>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <sstream>
#include <stack>
#include <thread>
#include <unordered_map>
#include <vector>

#include "TermStyle.hpp"

#define ENABLE_PERF_TRACE
#ifdef ENABLE_PERF_TRACE
#define PERF_TRACE(X) glr::PerfTracer perf_##__COUNTER__(X);
#define THREAD_PERF_TRACE(X) glr::PerfTracer perf_##__COUNTER__(X, true);
#else
#define PERF_TRACE(X)
#endif

/*!
 * @author: Jiawei Liu.
 */
namespace glr
{

class PerfTracer{
public: // Types
    using clk_t =  std::chrono::high_resolution_clock;

public:  // Public Functions.
    ///
    /// \param n The name string.
    /// \param specify_thread Specify the name with thread hash id.
    PerfTracer(std::string n, bool specify_thread = false);

    /// DeConstructor. Fill the table and register report function at exit.
    ~PerfTracer();


private: // Private Functions.
    /// Write results to the final report table.
    void _write_table();


public:  // Public Data
    static std::ostream&   output_stream;
private: // Private Data
    clk_t::time_point      m_tp;
    std::string            m_name;
};

}