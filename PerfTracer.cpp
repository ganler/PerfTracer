//
// Created by ganler on 3/11/20.
//

#include "PerfTracer.hpp"


namespace glr
{

constexpr int proportion_precision = 3;

std::ostream& glr::PerfTracer::output_stream{std::cout};


struct ResultType{
    size_t                   called_times = 0;
    double                   accumulated_time = 0.;
    std::vector<std::string> sons{};
};

struct OutputType{
    std::string name, sum, times, avg;
    std::vector<std::pair<std::string, double>> spawned_calls_this_thread;
};

using summary_table_t = std::unordered_map<std::string, ResultType>;


static std::stack<std::reference_wrapper<std::string>>& _thread_trace_stack() {
    thread_local std::stack< std::reference_wrapper<std::string>> stack;
    return stack;
}

static summary_table_t& _summary_table() {
    static summary_table_t table;
    return table;
}


static void _summary() {
    using table_value_ref_t = std::reference_wrapper<summary_table_t::value_type>;

    // Greeting
    auto greeting_style = GREEN.style({Style ::BOLD, Style ::ITALIC});

    greeting_style.line();
    auto& output_stream = PerfTracer::output_stream;
    output_stream << greeting_style << ">>> [SUMMARY]" <<
                  CLEAN << "::" << YELLOW << "[rank by average]\n";
    greeting_style.line();

    std::vector<table_value_ref_t >
            ref_vector(_summary_table().begin(), _summary_table().end());

    auto avg_from_view = [](const table_value_ref_t& v){
        return v.get().second.accumulated_time / v.get().second.called_times;
    };

    std::sort(ref_vector.begin(), ref_vector.end(), [&avg_from_view](const table_value_ref_t& l, const table_value_ref_t& r){
        // Less Fashion.
        return avg_from_view(l) > avg_from_view(r);
    });

    // Fields.
    int max_len_one_line = get_term_length();
    int ID = 2, NAME = 4, SUM = 3, TIMES = 5, AVERAGE = 7, SPAWNED_CALL_THIS_THREAD=24;

    ++ID, ++NAME, ++SUM, ++TIMES, ++AVERAGE, ++SPAWNED_CALL_THIS_THREAD; // 1 more byte space;

    // Result Vector
    std::vector<OutputType> outputs;
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

        auto& ref_sons = n.get().second.sons;
        std::sort(ref_sons.begin(), ref_sons.end(), [](const std::string& l, const std::string& r){
            return _summary_table()[l].accumulated_time > _summary_table()[r].accumulated_time;
        });
        size_t esitmated_sctt_len = 0;
        for(auto&& son : ref_sons)
        {
            std::stringstream ss;
            auto res = _summary_table().find(son);
            table_value_ref_t v = std::ref (*res);
            auto it = std::lower_bound(ref_vector.cbegin(), ref_vector.cend(), v, [&avg_from_view](auto&& l, auto&& r){
                // Less Fashion.
                return avg_from_view(l) > avg_from_view(r);
            }); // Log2(N)
            int id = std::distance(ref_vector.cbegin(), it);
            double proportion = avg_from_view(*res) / this_average;
            ss << " {ID:" << std::to_string(id + 1) << "}@" << std::setprecision(proportion_precision) << proportion * 100 << '%';
            sctt.emplace_back(ss.str(), proportion);
            esitmated_sctt_len += sctt.back().first.size();
        }

        NAME = std::max(NAME, (int)outputs.back().name.size());
        SUM = std::max(SUM, (int)outputs.back().sum.size());
        TIMES = std::max(TIMES, (int)outputs.back().times.size());
        AVERAGE = std::max(AVERAGE, (int)outputs.back().avg.size());

        SPAWNED_CALL_THIS_THREAD = std::max(SPAWNED_CALL_THIS_THREAD, (int)esitmated_sctt_len);
    }
    int left_for_last_field = max_len_one_line - ID - NAME - SUM - TIMES - AVERAGE;
    SPAWNED_CALL_THIS_THREAD = std::max(SPAWNED_CALL_THIS_THREAD, left_for_last_field);
    constexpr char no_call_prompt[] = " No spawned calls in this thread ";
    SPAWNED_CALL_THIS_THREAD = std::max(SPAWNED_CALL_THIS_THREAD, (int)sizeof(no_call_prompt));

    size_t lines = ref_vector.size();
    int at_least_chars_for_id = 1;
    while((lines /= 10) != 0)
        ++at_least_chars_for_id;
    ID = std::max(ID, at_least_chars_for_id);

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
    color_switch[select++].style({Style::ITALIC, Style::BOLD, Style::UNDERLINE}).string_field(ID, "ID", output_stream);
    color_switch[select++].style({Style::ITALIC, Style::BOLD, Style::UNDERLINE}).string_field(NAME, "NAME", output_stream);
    color_switch[select++].style({Style::BOLD, Style::UNDERLINE}).string_field(SUM, "SUM", output_stream);
    color_switch[select++].style({Style::BOLD, Style::UNDERLINE}).string_field(TIMES, "TIMES", output_stream);
    color_switch[select++].style({Style::BOLD, Style::UNDERLINE}).string_field(AVERAGE, "AVERAGE", output_stream);
    color_switch[select++].style({Style::BOLD, Style::UNDERLINE}).string_field(SPAWNED_CALL_THIS_THREAD, "SPAWNED_CALL_THIS_THREAD", output_stream);
    output_stream << CLEAN << '\n';

    int id = 1;
    for(auto&& line : outputs) {
        select = 0;
        color_switch[select++].style({Style::REVERSE, Style::ITALIC, Style::BOLD}).string_field(ID, std::to_string(id++), output_stream);
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

static std::mutex& _perf_tracer_mutex() {
    static std::mutex mu;
    return mu;
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
    {
        std::lock_guard<std::mutex> l{_perf_tracer_mutex()};
        auto& son_vec = _summary_table()[_thread_trace_stack().top().get()].sons;
        auto it = std::find(son_vec.begin(), son_vec.end(), m_name);
        if(son_vec.cend() == it)
            son_vec.push_back(m_name);
    }
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

    std::lock_guard<std::mutex> l{_perf_tracer_mutex()};
    ResultType& result = _summary_table()[m_name];
    ++result.called_times;
    result.accumulated_time += period;
}

}