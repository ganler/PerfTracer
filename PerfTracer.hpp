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
#define PERF_TRACE(X) pt::PerfTracer perf_##__COUNTER__(X);
#define THREAD_PERF_TRACE(X) pt::PerfTracer perf_##__COUNTER__(X, true);
#else
#define PERF_TRACE(X)
#endif

/*!
 * @author: Jiawei Liu.
 */
namespace pt
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


// Impl. Backup.
//class SingletonReporter{
//public:
//  SingletonReporter() = default;
//
//  SingletonReporter(const SingletonReporter&) = delete;
//  SingletonReporter(SingletonReporter&&)      = delete;
//  SingletonReporter& operator=(const SingletonReporter&) = delete;
//  SingletonReporter& operator=(SingletonReporter&&)      = delete;
//
//  ///
//  SingletonReporter& get() {
//    static SingletonReporter singleton{};
//    return singleton;
//  }
//
//  ///
//  static void report(){
//    std::cout << '\n' << pt::GREEN.style({pt::Style::BOLD, pt::Style::REVERSE}) << "[PERF ENDS] >>> "
//              << pt::CYAN << " Here's the result :-)\n";
//    const auto& items = _report_vector();
//    auto style = pt::PURPLE.style({pt::Style::BOLD});
//    for(auto&& item : items)
//    {
//      style.line(0, '=');
//      std::cout << CLEAN << item << '\n';
//      style.line(0, '=');
//      std::cout << '\n';
//    }
//  }
//
//  ///
//  /// \param str
//  static void add_report_item(std::string str){
//    {
//      static std::mutex mu;
//      std::lock_guard<std::mutex> l{mu};
//      _report_vector().push_back(std::move(str));
//    }
//
//    // The order cannot change, see: https://zh.cppreference.com/w/cpp/utility/program/atexit
//    static bool called = false;
//    if(!called) {
//      called = true; // Thread safety is not important here.
//      std::atexit(report); // Funcs can be registered more than once.
//    }
//  }
//
//private:
//  ///
//  /// \return
//  static std::vector<std::string>& _report_vector() {
//    static std::vector<std::string> vec;
//    return vec;
//  }
//};




// /*!
// * Performance tracer that can be used to trace runtime in a tree fashion(perf
// * tree). This PerfTracer is also thread safe. But if you spawn a new thread,
// * there will be a new perf tree.
// * WARNING: DO NOT USE PerfTracer in the same stack frame.
// */
//class PerfTracer
//{
//public:
//  using clk_t =  std::chrono::high_resolution_clock;
//  using period_t = std::array<clk_t::time_point, 2>;
//
//  ///
//  /// \param name
//  PerfTracer(std::string str): name(std::move(str)), period{clk_t::now(),} {
//    _node_stack().push(this);
//  }
//
//  PerfTracer(std::string&& n, period_t&& p, std::vector<std::unique_ptr<PerfTracer>>&& v, const PerfTracer* f)
//      : name(std::move(n)), period(std::move(p)), sons(std::move(v)), dad(f){}
//
//  ///
//  ~PerfTracer() {
//    if(dad != nullptr) // If you have dad, you don't need to look for another dad.
//      return;
//
//    this->set_end_time(); // Data prepared!
//    _node_stack().pop();
//    if (_node_stack().empty()) // Check if I'm the last node in the stack.
//    {
//      SingletonReporter reporter;
//      reporter.add_report_item(this->summary_perf_tree());
//      // Task finished, it's time to commit suicide.
//      // All right, I used std::unique_ptr to help.
//    } else { // Submit my data to my father.
//      // Who's my dad? The last node!
//      // Creat a new copy of data;
//      this->dad = _node_stack().top();
//      auto node = std::make_unique<PerfTracer>(std::move(name), std::move(period), std::move(sons), dad);
//      _node_stack().top()->sons.push_back(std::move(node));
//    }
//  }
//
//  std::string summary_perf_tree(bool sort_results = false) const {
//    const size_t uniform_term_len = get_term_length() * 0.75;
//    std::ostringstream ss;
//
//    constexpr std::array<TermStyle<0>, 8> color_switch{
//        BLACK,
//        RED,
//        GREEN,
//        YELLOW,
//        BLUE,
//        PURPLE,
//        CYAN,
//        WHITE,
//    };
//    size_t select = 0;
//    constexpr char connect[] {" --> "};
//
//    // All right, BFS.
//    std::queue<const_pointer > queue;
//
//    // Process root first.
//    auto&& root_style = color_switch[select++];
//    ss << root_style.style(Style::REVERSE) << "root" << connect << this->name << '[' << root_style.style({Style::REVERSE, Style::BOLD}) << this->duration() << " ms @ 100%]";
//
//    std::vector<const_pointer > line_process;
//    for(auto&& n : sons)
//      queue.push(n.get());
//    queue.push(nullptr); // Note.
//
//    while(!(queue.size() == 1 && queue.front() == nullptr))
//    {
//      // Process.
//      auto node = queue.front();
//      queue.pop();
//
//      if(node == nullptr) // Time to process a line;
//      {
//        double duratoin_sum = std::accumulate(line_process.begin(), line_process.end(), 0.,
//                                              [](double& init, const_pointer x){ return init + x->duration(); });
//
//        if(sort_results)
//          std::sort(line_process.begin(), line_process.end(), [](auto&& l, auto&& r){
//            return l->duration() > r->duration();
//          });
//
//        size_t bytes_left = uniform_term_len;
//
//        for(const auto& n : line_process)
//        {
//          double proportion = n->duration() / duratoin_sum;
//
//          auto&& style = color_switch[select++];
//          size_t allocated_length = std::max();
//          // TOOD.
//          ss << style.style(Style::REVERSE) << n->name;
//        }
//
//        std::cout << '\n';
//
//        queue.push(nullptr); // Add a note.
//        line_process.clear();
//      } else
//        line_process.push_back(node);
//
//
//      // Receive.
//      for(const auto& n : node->sons)
//        queue.push(n.get());
//    }
//
//    return ss.str();
//  }
//
//  double duration() const noexcept {
//    return std::chrono::duration<double, std::milli>(period[1]-period[0]).count();
//  }
//
//private:
//  using pointer       = PerfTracer*;
//  using const_pointer = const PerfTracer*;
//
//  ///
//  /// \return
//  static std::stack<pointer> & _node_stack() {
//    thread_local std::stack<pointer> _node_stack {}; // For thread safe.
//    return _node_stack;
//  }
//
//  void set_end_time(){
//    period[1] = clk_t::now();
//  }
//
//  std::string                               name;
//  period_t                                  period;
//  std::vector<std::unique_ptr<PerfTracer>>  sons;
//  const_pointer                             dad = nullptr;
//};

}