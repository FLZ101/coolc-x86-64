#include "util.hh"

#include <sstream>

namespace util {

std::string escape_string(const std::string &s) {
  std::ostringstream o;
  for (auto ch : s) {
    switch (ch) {
    case '\n':
      o << "\\n";
      break;
    case '\t':
      o << "\\t";
      break;
    case '\f':
      o << "\\f";
      break;
    case '\b':
      o << "\\b";
      break;
    case '"':
      o << "\\\"";
    default:
      o << ch;
    }
  }
  return o.str();
}

} /* namespace util */
