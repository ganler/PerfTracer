#pragma once

#include <array>
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <cassert>

#include<sys/ioctl.h>
#include<unistd.h>

/*!
 * @author: Jiawei Liu.
 */
namespace pt
{

// https://en.wikipedia.org/wiki/ANSI_escape_code
enum class Style : uint8_t {
  NORMAL        = 0,
  BOLD          = 1,
  FAINT         = 2,
  ITALIC        = 3,
  UNDERLINE     = 4,
  SLOW_BLINK    = 5,
  RAPID_BLINK   = 6, // Not widely supported.
  REVERSE       = 7,
  CONCEAL       = 8, // Not widely supported.
  CROSS_OUT     = 9
};


enum class Color : uint8_t {
  CLEAN    = 0,
  BLACK    = 30,
  RED      = 31,
  GREEN    = 32,
  YELLOW   = 33,
  BLUE     = 34,
  PURPLE   = 35,
  CYAN     = 36,
  WHITE    = 37,
  DEFAULT  = 39,
};

inline size_t get_term_length(unsigned short int max_len = 256) {
  winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return std::min(w.ws_col, max_len);
}

/// Class to help stylize the stream outputs.
template <size_t MAX_STYLE_NUM=0>
class TermStyle {
public:
  static constexpr size_t max_style_num = MAX_STYLE_NUM;

  /// Constructor.
  /// \param color The style colors. See class `pt::Color`.
  /// \param styles The styles. See class `pt::Style`.
  constexpr TermStyle(Color color, std::array<Style, MAX_STYLE_NUM> styles = {})
      : m_color_code(static_cast<uint8_t>(color)), style_array(styles){}

  ///
  /// \return A new TermStyle class with background colored.
  constexpr inline TermStyle background() const noexcept {
    return termcolor_from_int(m_color_code + 10);
  }

  ///
  /// \return A new TermStyle class with brighter font colored.
  constexpr inline TermStyle bright() const noexcept {
    return termcolor_from_int(m_color_code + 60);
  }

  ///
  /// \param s_ Single Style object to add the styles of TermStyle.
  /// \return A new TermStyle class with appended style.
  inline TermStyle<MAX_STYLE_NUM+1> style(Style s_) const noexcept {
    Style sarr[]{s_};
    return style(sarr);
  }

  ///
  /// \tparam N The number of styles to be appended.
  /// \param sl Array list of styles to be appended.
  /// \return A new TermStyle class with appended styles.
  template <size_t N>
  inline TermStyle<MAX_STYLE_NUM+N> style(const Style (&sl)[N]) const noexcept {
    TermStyle<MAX_STYLE_NUM+N> t(this->color());
    std::copy(this->style_array.begin(), this->style_array.end(), t.style_array.begin());
    std::copy(sl, sl + N, t.style_array.begin() + MAX_STYLE_NUM);
    return t;
  }

  /// Get current color enum.
  /// \return current color enum.
  constexpr inline Color color() const noexcept {
    return static_cast<Color>(m_color_code);
  }

  /// Draw a line in the terminal.
  /// \param n The line length. (The default value is 0, means fitting the current term length.
  /// \param pattern The replicate line character.
  /// \param os The output stream.
  inline void line(int n = 0, char pattern = '-', std::ostream& os = std::cout) const {
    if (0 == n)
      n = get_term_length() - 2;

    n = std::min(n, 256);

    os << (*this) << '+';
    padding(n, pattern, os);
    os << '+' << TermStyle<0>(Color::CLEAN) << '\n';
  }

  /// Drawing replicas.
  /// \param np The number of replicas.
  /// \param pattern The replica character.
  /// \param os The output stream.
  inline void padding(int np, char pattern = ' ', std::ostream& os = std::cout) const {
    for (int i = 0; i < np; ++i)
      os << pattern;
  };

  /// Draw a auto-fixed string field.
  /// \param max_len The maximum length of this field.
  /// \param res The string field content.
  /// \param os The output stream.
  inline void  string_field(size_t max_len, const std::string& res, std::ostream& os = std::cout) const {
    if (max_len == 0)
      max_len = res.size();
    assert(max_len >= res.size());
    os << (*this) << res;
    padding(max_len - res.size());
  };

  /// As a C++ programmer, you all know this:-)
  /// \param os
  /// \param term_color
  /// \return
  friend inline std::ostream& operator<<(std::ostream& os, const TermStyle & term_color) {
    os << "\033[0m\033[" << static_cast<int>(term_color.m_color_code) ;

    for(auto s : term_color.style_array)
      if(s == Style::NORMAL)
        break;
      else
        os << ';' << static_cast<int>(s);

    return os << 'm';
  }

  std::array<Style, MAX_STYLE_NUM> style_array;

private:
  constexpr inline TermStyle termcolor_from_int(uint8_t c) const noexcept {
    return TermStyle{static_cast<Color>(c)};
  }
  uint8_t m_color_code;
};


constexpr TermStyle<0> RED(Color::RED), BLUE(Color::BLUE), GREEN(Color::GREEN),
    BLACK(Color::BLACK), YELLOW(Color::YELLOW), PURPLE(Color::PURPLE),
    CYAN(Color::CYAN), WHITE(Color::WHITE), DEF(Color::DEFAULT), CLEAN(Color::CLEAN);


}
