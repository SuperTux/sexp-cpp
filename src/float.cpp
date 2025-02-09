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
#include <charconv>
#include <limits>
#include <sstream>

namespace sexp {

float string2float(const std::string& text)
{
  std::istringstream iss(text);
  iss.imbue(std::locale::classic());
  float result;
  iss >> result;
  assert(!iss.eof());
  return result;
}

void float2string(std::ostream& os, float value)
{
  std::ostringstream oss();
  oss.imbue(std::locale::classic());
  oss << value;
  os << oss.str();
}

} // namespace sexp

/* EOF */
