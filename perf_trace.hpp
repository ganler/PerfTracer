//
// Created by ganler on 3/11/20.
//


/*!
 * @author: Jiawei Liu.
 */

#pragma once

#include <chrono>
#include <iostream>
#include <string>

#define ENABLE_PERF_TRACE
#ifdef ENABLE_PERF_TRACE
#define PERF_TRACE(X) glr::perf_trace perf_##__COUNTER__(X);
#define THREAD_PERF_TRACE(X) glr::perf_trace perf_##__COUNTER__(X, true);

namespace glr
{

class perf_trace{
public: // Types
    using clk_t =  std::chrono::high_resolution_clock;

public:  // Public Functions.
    ///
    /// \param n The name string.
    /// \param specify_thread Specify the name with thread hash id.
    perf_trace(std::string n, bool specify_thread = false);

    /// DeConstructor. Fill the table and register report function at exit.
    ~perf_trace();


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

#else
#define PERF_TRACE(X)
#endif
