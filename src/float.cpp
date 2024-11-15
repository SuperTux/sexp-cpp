// SExp - A S-Expression Parser for C++
// Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//               2018 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "float.hpp"

#include <cassert>
#if USE_FAST_FLOAT
# include <fast_float/fast_float.h>
using fast_float::from_chars;
using fast_float::from_chars_result;
#else
# include <charconv>
using std::from_chars;
using std::from_chars_result;
#endif
#include <limits>
#include <sstream>

namespace sexp {

float string2float(const std::string& text)
{
  char const* start = text.data();

  // A leading + (e.g. "+5") is not accepted by from_chars(), so skip it
  if (!text.empty() && text[0] == '+') {
    start += 1;
  }

  float result;
  [[maybe_unused]] auto err = from_chars(start, text.data() + text.size(), result);
  assert(err.ec == std::errc());
  return result;
}

void float2string(std::ostream& os, float value)
{
#ifdef SEXP_USE_CXX17
  constexpr size_t len = std::numeric_limits<float>::digits10 + 1;
  char buffer[len];
  auto result = std::to_chars(buffer, buffer + len, value);
  assert(result.ec == std::errc());
  os.write(buffer, result.ptr - buffer);
#else
  auto precision = os.precision(std::numeric_limits<float>::digits10 + 1);
#  ifdef SEXP_USE_LOCALE
  const auto& loc = os.getloc();
  os.imbue(std::locale::classic());
  os << value;
  os.imbue(loc);
#  else
  os << value;
#  endif
  os.precision(precision);
#endif
}

} // namespace sexp

/* EOF */
