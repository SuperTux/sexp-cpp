// SExp - A S-Expression Parser for C++
// Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//               2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "sexp/parser.hpp"

#include <sstream>
#include <stdexcept>
#include <string.h>
#include <iostream>

#include "float.hpp"

namespace sexp {

Value
Parser::from_string(std::string const& str, bool use_arrays, int depth)
{
  std::istringstream is(str);
  return from_stream(is, use_arrays, depth);
}

Value
Parser::from_stream(std::istream& stream, bool use_arrays, int depth)
{
  Lexer lexer(stream, use_arrays);
  Parser parser(lexer, depth);
  Value result = parser.read();
  if (parser.m_token != Lexer::TOKEN_EOF)
  {
    parser.parse_error("trailing garbage in stream");
  }
  return result;
}

std::vector<Value>
Parser::from_string_many(std::string const& str, bool use_arrays, int depth)
{
  std::istringstream is(str);
  return from_stream_many(is, use_arrays, depth);
}

std::vector<Value>
Parser::from_stream_many(std::istream& stream, bool use_arrays, int depth)
{
  Lexer lexer(stream, use_arrays);
  Parser parser(lexer, depth);
  return parser.read_many();
}

Parser::Parser(Lexer& lexer, int depth) :
  m_lexer(lexer),
  m_depth(depth),
  m_token(m_lexer.get_next_token()),
  m_current_depth(-1)
{
}

Parser::~Parser()
{
}

void
Parser::parse_error(const char* msg) const
{
  std::stringstream emsg;
  emsg << "Parse Error at line " << m_lexer.get_line_number()
       << ": " << msg;
  throw std::runtime_error(emsg.str());
}

std::vector<Value>
Parser::read_many()
{
  std::vector<Value> results;
  while(m_token != Lexer::TOKEN_EOF)
  {
    results.emplace_back(read());
  }
  return results;
}

Value
Parser::read()
{
  Value result;
  int line_number = m_lexer.get_line_number();

  switch(m_token)
  {
    case Lexer::TOKEN_OPEN_PAREN:
      m_token = m_lexer.get_next_token();
      if (m_token == Lexer::TOKEN_CLOSE_PAREN)
        break;

      ++m_current_depth;
      // If a depth is specified, do not parse objects below it.
      if (m_depth >= 0 && m_current_depth > m_depth)
      {
        while (m_token != Lexer::TOKEN_CLOSE_PAREN)
        {
          if (m_token == Lexer::TOKEN_OPEN_PAREN)
            read(); // Recursively go through the tokens of child objects.
          else
            m_token = m_lexer.get_next_token();
        }
      }
      else
      {
        result = Value::cons(read(), Value::nil());
        Value* cur = &result;
        while (m_token != Lexer::TOKEN_CLOSE_PAREN)
        {
          if (m_token == Lexer::TOKEN_DOT)
          {
            m_token = m_lexer.get_next_token();

            Value val = read();
            // Do not include values that would be nil for being below the specified depth.
            if (m_depth < 0 || m_current_depth != m_depth || !val.is_nil())
              cur->set_cdr(std::move(val));

            if (m_token != Lexer::TOKEN_CLOSE_PAREN)
            {
              parse_error("Expected ')'");
            }
            break;
          }
          else
          {
            Value val = read();
            // Do not include values that would be nil for being below the specified depth.
            if (m_depth < 0 || m_current_depth != m_depth || !val.is_nil())
            {
              cur->set_cdr(Value::cons(std::move(val), Value::nil()));
              cur = &cur->get_cdr();
            }
          }
        }
      }
      --m_current_depth;
      break;

    case Lexer::TOKEN_SYMBOL:
      result = Value::symbol(m_lexer.get_string());
      break;

    case Lexer::TOKEN_STRING:
      result = Value::string(m_lexer.get_string());
      break;

    case Lexer::TOKEN_INTEGER:
      result = Value::integer(std::stoi(m_lexer.get_string()));
      break;

    case Lexer::TOKEN_REAL:
      result = Value::real(string2float(m_lexer.get_string()));
      break;

    case Lexer::TOKEN_TRUE:
      result = Value::boolean(true);
      break;

    case Lexer::TOKEN_FALSE:
      result = Value::boolean(false);
      break;

    case Lexer::TOKEN_ARRAY_START:
      m_token = m_lexer.get_next_token();
      if (m_token == Lexer::TOKEN_CLOSE_PAREN)
        break;

      ++m_current_depth;
      // If a depth is specified, do not parse arrays below it.
      if (m_depth >= 0 && m_current_depth > m_depth)
      {
        while (m_token != Lexer::TOKEN_CLOSE_PAREN)
        {
          if (m_token == Lexer::TOKEN_ARRAY_START)
          {
            read(); // Recursively go through the tokens of child arrays.
          }
          else
          {
            // If the array is just below the specified depth, but contains a translatable string, parse it.
            if (m_current_depth - 1 == m_depth &&
                m_token == Lexer::TOKEN_SYMBOL && m_lexer.get_string() == "_")
            {
              m_token = m_lexer.get_next_token();
              if (m_token == Lexer::TOKEN_STRING)
              {
                const std::string str = m_lexer.get_string();
                m_token = m_lexer.get_next_token();
                if (m_token == Lexer::TOKEN_CLOSE_PAREN)
                {
                  result = Value::array({ Value::symbol("_"), Value::string(str) });
                  break;
                }
              }
            }
            else
            {
              m_token = m_lexer.get_next_token();
            }
          }
        }
      }
      else
      {
        std::vector<Value> arr;
        do
        {
          Value val = read();
          // Do not include values that would be nil for being below the specified depth.
          if (m_depth < 0 || m_current_depth != m_depth || !val.is_nil())
            arr.push_back(std::move(val));
        }
        while (m_token != Lexer::TOKEN_CLOSE_PAREN);
        result = Value::array(std::move(arr));
      }
      --m_current_depth;
      break;

    case Lexer::TOKEN_EOF:
      parse_error("Unexpected EOF.");
      break;

    case Lexer::TOKEN_CLOSE_PAREN:
      parse_error("Unexpected ')'.");
      break;

    case Lexer::TOKEN_DOT:
      parse_error("Unexpected '.'.");
      break;

    default:
      assert(false && "this should never happen");
      break;
  }

  m_token = m_lexer.get_next_token();

  result.set_line(line_number);
  return result;
}

} // namespace sexp

/* EOF */
