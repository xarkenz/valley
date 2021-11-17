#pragma once

#include "tokens.hpp"
#include "util.hpp"


namespace valley {

  class TokenIterator {
  private:
    PushBackStream& _stream;
    Token _current;
  
  public:
    TokenIterator(PushBackStream& Stream);

    const Token& operator*() const;
    const Token* operator->() const;
    TokenIterator& operator++();
    explicit operator bool() const;

    void stepBack(Token t);
  };

}