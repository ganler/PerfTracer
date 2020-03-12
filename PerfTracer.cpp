//
// Created by ganler on 3/11/20.
//

#include "PerfTracer.hpp"


namespace pt
{

std::ostream& pt::PerfTracer::output_stream{std::cout};

static std::stack<std::reference_wrapper<std::string>>& _thread_trace_stack() {
  thread_local std::stack< std::reference_wrapper<std::string>> stack;
  return stack;
}

static PerfTracer::summary_table_t& _summary_table() {
  static PerfTracer::summary_table_t table;
  return table;
}


static void _summary() {
  using value_view_t = std::reference_wrapper<const PerfTracer::summary_table_t::value_type>;

  // Greeting
  auto greeting_style = GREEN.style({Style ::BOLD, Style ::ITALIC});

  greeting_style.line();
  auto& output_stream = PerfTracer::output_stream;
  output_stream << greeting_style << ">>> [SUMMARY]" <<
                CLEAN << "::" << YELLOW << "[rank by average]\n";
  greeting_style.line();

  std::vector<value_view_t >
      ref_vector(_summary_table().cbegin(), _summary_table().cend());

  auto avg_from_view = [](value_view_t v){
    return v.get().second.accumulated_time / v.get().second.called_times;
  };

  std::sort(ref_vector.begin(), ref_vector.end(), [&avg_from_view](const value_view_t l, const value_view_t& r){
    // Less Fashion.
    return avg_from_view(l) > avg_from_view(r);
  });

  // Fields.
  constexpr double factor = 0.8;
  int max_len_one_line = get_term_length() * factor;
  int NAME = 4, SUM = 3, TIMES = 5, AVERAGE = 7, SPAWNED_CALL_THIS_THREAD=24;

  ++NAME, ++SUM, ++TIMES, ++AVERAGE, ++SPAWNED_CALL_THIS_THREAD; // 1 more byte space;

  // Result Vector
  std::vector<PerfTracer::OutputType> outputs;
  outputs.reserve(ref_vector.size());

  for(auto&& n : ref_vector)
  {
    double this_average = avg_from_view(n);

    outputs.push_back({
                          n.get().first + ' ',
                          std::to_string(n.get().second.accumulated_time) + " ms ",
                          std::to_string(n.get().second.called_times) + "# ",
                          std::to_string(this_average) + " ms "});

    auto& sctt = outputs.back().spawned_calls_this_thread;
    sctt.reserve(n.get().second.sons.size());

    size_t esitmated_sctt_len = 0;
    for(auto&& son : n.get().second.sons)
    {
      std::stringstream ss;
      const auto& res = _summary_table()[son];
      double proportion = (res.accumulated_time / res.called_times) / this_average;
      ss << " {" << son << "}@" << std::setprecision(4) << proportion * 100 << '%';
      sctt.emplace_back(ss.str(), proportion);
      esitmated_sctt_len += sctt.back().first.size();
    }

    NAME = std::max(NAME, (int)outputs.back().name.size());
    SUM = std::max(SUM, (int)outputs.back().sum.size());
    TIMES = std::max(TIMES, (int)outputs.back().times.size());
    AVERAGE = std::max(AVERAGE, (int)outputs.back().avg.size());

    SPAWNED_CALL_THIS_THREAD = std::max(SPAWNED_CALL_THIS_THREAD, (int)esitmated_sctt_len);
  }
  SPAWNED_CALL_THIS_THREAD = std::max(SPAWNED_CALL_THIS_THREAD, max_len_one_line - NAME - SUM - TIMES - AVERAGE);
  constexpr char no_call_prompt[] = " No spawned calls in this thread ";
  SPAWNED_CALL_THIS_THREAD = std::max(SPAWNED_CALL_THIS_THREAD, (int)sizeof(no_call_prompt));


  // Output Results
  constexpr std::array<TermStyle<0>, 8> color_switch{
      BLACK,
      RED,
      GREEN,
      YELLOW,
      BLUE,
      PURPLE,
      CYAN,
      WHITE,
  };

  size_t select = 0;
  color_switch[select++].style({Style::ITALIC, Style::BOLD, Style::UNDERLINE}).string_field(NAME, "NAME", output_stream);
  color_switch[select++].style({Style::BOLD, Style::UNDERLINE}).string_field(SUM, "SUM", output_stream);
  color_switch[select++].style({Style::BOLD, Style::UNDERLINE}).string_field(TIMES, "TIMES", output_stream);
  color_switch[select++].style({Style::BOLD, Style::UNDERLINE}).string_field(AVERAGE, "AVERAGE", output_stream);
  color_switch[select++].style({Style::BOLD, Style::UNDERLINE}).string_field(SPAWNED_CALL_THIS_THREAD, "SPAWNED_CALL_THIS_THREAD", output_stream);
  output_stream << CLEAN << '\n';

  for(auto&& line : outputs) {
    select = 0;
    color_switch[select++].style({Style::REVERSE, Style::ITALIC, Style::BOLD}).string_field(NAME, line.name, output_stream);
    color_switch[select++].style(Style::REVERSE).string_field(SUM, line.sum, output_stream);
    color_switch[select++].style(Style::REVERSE).string_field(TIMES, line.times, output_stream);
    color_switch[select++].style(Style::REVERSE).string_field(AVERAGE,line.avg, output_stream);

    // Colorize the Following Part.
    if(line.spawned_calls_this_thread.empty())
    {
      auto style = RED.style({Style::FAINT, Style::CROSS_OUT});
      int space = SPAWNED_CALL_THIS_THREAD - (int)sizeof(no_call_prompt);
      output_stream << style;
      style.padding(std::max(0, space/2), ' ', output_stream);
      output_stream << no_call_prompt;
      style.padding(std::max(0, space - space/2), ' ', output_stream);
    } else{
      for (auto&& it : line.spawned_calls_this_thread) {
        color_switch[(++select) % color_switch.size()]
            .style(Style::REVERSE)
            .string_field(
                std::max((size_t)(it.second * SPAWNED_CALL_THIS_THREAD),
                         it.first.size()),
                it.first, output_stream);
      }
    }

    output_stream << CLEAN << '\n';
  }
  greeting_style.line();
}

static void _register_report() {
  // The order cannot change, see: https://zh.cppreference.com/w/cpp/utility/program/atexit
  // `std::atexit` can be registered more than once, there's no need to help with the thread safety.
  static bool flag = true;
  if(flag){
    std::atexit(_summary);
    flag = false;
  }
}


PerfTracer::PerfTracer(std::string n, bool specify_thread)
    : m_name(std::move(n) + ((specify_thread) ? "-Thread@" + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())) : ""))
{
  if(!_thread_trace_stack().empty()) // If there's a dad, push me a son.
    _summary_table()[_thread_trace_stack().top().get()].sons.push_back(
        m_name);
  _thread_trace_stack().push(std::ref(m_name)); // Push stack.
  m_tp = clk_t::now();
}

PerfTracer::~PerfTracer() {
  _write_table();
  _register_report();
}


void PerfTracer::_write_table() {
  double period = std::chrono::duration<double, std::milli>(clk_t::now() - m_tp).count();
  _thread_trace_stack().pop();

  static std::mutex mu;
  std::lock_guard<std::mutex> l{mu};
  ResultType& result = _summary_table()[m_name];
  ++result.called_times;
  result.accumulated_time += period;
}

}