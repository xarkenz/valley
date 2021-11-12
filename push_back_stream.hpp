#ifndef PUSH_BACK_STREAM_HPP
#define PUSH_BACK_STREAM_HPP

#include <functional>
#include <stack>

namespace valley {
  using get_character = std::function<int()>;

  class push_back_stream {
    private:
      const get_character& _input;
      std::stack<int> _stack;
      size_t _line_number;
      size_t _char_index;
      
    public:
      push_back_stream(const get_character& input);

      int operator()();

      void push_back(int c);

      size_t line_number() const;
      size_t char_index() const;
  };
}

#endif