#include "util.hpp"


namespace valley {

  PushBackStream::PushBackStream(const CharGetter& input):
    _input(input),
    _line_number(0),
    _char_index(0)
  {
  }

  int PushBackStream::operator()() {
    int ret = -1;
    if (_stack.empty()) {
      ret = _input();
    } else {
      ret = _stack.top();
      _stack.pop();
    }
    if (ret == '\n') {
      ++_line_number;
    }
    ++_char_index;
    return ret;
  }

  void PushBackStream::pushBack(int c) {
    _stack.push(c);
    if (c == '\n') {
      --_line_number;
    }
    --_char_index;
  }

  size_t PushBackStream::lineNumber() const {
    return _line_number;
  }

  size_t PushBackStream::charIndex() const {
    return _char_index;
  }
  
}