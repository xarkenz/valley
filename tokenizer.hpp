#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <functional>
#include <iostream>
#include <string_view>
#include <variant>

#include "push_back_stream.hpp"
#include "tokens.hpp"

namespace valley {
  class tokens_iterator {
    private:
      push_back_stream& _stream;
      token _current;
    
    public:
      tokens_iterator(push_back_stream& stream);

      const token& operator*() const;
      const token* operator->() const;
      tokens_iterator& operator++();
      explicit operator bool() const;

      void move_back(token t);
  };
}

#endif