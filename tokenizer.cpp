#include "tokenizer.hpp"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <stack>
#include <string>

#include "errors.hpp"
#include "push_back_stream.hpp"

namespace valley {
  namespace {
    enum struct char_type {
      eof,
      whitespace,
      alpha,
      numeric,
      other,
    };

    char_type get_char_type(int c) {
      if (c < 0) {
        return char_type::eof;
      }
      if (std::isspace(c)) {
        return char_type::whitespace;
      }
      if (std::isalpha(c) || c == '_') {
        return char_type::alpha;
      }
      if (std::isdigit(c)) {
        return char_type::numeric;
      }
      return char_type::other;
    }

    token fetch_word(push_back_stream& stream) {
      size_t line_number = stream.line_number();
      size_t char_index = stream.char_index();

      std::string word;
      int c = stream();
      bool is_number = std::isdigit(c) || c == '.';
      bool is_double = false;
      do {
        word.push_back(char(c));
        c = stream();
        if (is_number && c == '.') {
          if (!is_double) {
            is_double = true;
          } else {
            throw unexpected_error(std::string(1, char(c)), stream.char_index(), stream.line_number(), true);
          }
        }
      } while (get_char_type(c) == char_type::alpha || get_char_type(c) == char_type::numeric || (is_number && c == '.'));

      stream.push_back(c);

      if (std::optional<reserved_token> t = get_keyword(word)) {
        if (*t == reserved_token::kw_elif) {
          // A little hacky, interprets as 'else' and artificially adds 'if' to the stream to be the next token
          stream.push_back('f');
          stream.push_back('i');
          return token(reserved_token::kw_else, line_number, char_index);
        }
        return token(*t, line_number, char_index);
      } else if (is_double) {
        char* endptr;
        double raw_num = strtod(word.c_str(), &endptr);
        if (*endptr) {
          if (*endptr == 'F' || *endptr == 'f') {
            float num = raw_num;
            return token(num, line_number, char_index);
          } else {
            size_t remaining = word.length() - (endptr - word.c_str());
            throw unexpected_error(std::string(1, char(*endptr)), stream.line_number(), stream.char_index() - remaining, false);
          }
        }
        return token(raw_num, line_number, char_index);
      } else if (is_number) {
        char* endptr;
        long int raw_num = strtol(word.c_str(), &endptr, 10);
        if (*endptr) {
          if (*endptr == 'B' || *endptr == 'b') {
            if (raw_num >= INT8_MIN && raw_num <= INT8_MAX) {
              int8_t num = raw_num;
              return token(num, line_number, char_index);
            } else {
              throw syntax_error("integer value out of range for type 'byte' (-2^7 to 2^7-1).", line_number, char_index, word.length());
            }
          } else if (*endptr == 'S' || *endptr == 's') {
            if (raw_num >= INT16_MIN && raw_num <= INT16_MAX) {
              int16_t num = raw_num;
              return token(num, line_number, char_index);
            } else {
              throw syntax_error("integer value too large for type 'short' (-2^15 to 2^15-1).", line_number, char_index, word.length());
            }
          } else if (*endptr == 'L' || *endptr == 'l') {
            if (raw_num >= INT64_MIN && raw_num <= INT64_MAX) {
              int64_t num = raw_num;
              return token(num, line_number, char_index);
            } else {
              throw syntax_error("integer value too large for type 'long' (-2^63 to 2^63-1).", line_number, char_index, word.length());
            }
          } else if (*endptr != 'I' && *endptr != 'i') {
            size_t remaining = word.length() - (endptr - word.c_str());
            throw unexpected_error(std::string(1, char(*endptr)), stream.line_number(), stream.char_index() - remaining, word.length());
          }
        }
        if (raw_num >= INT32_MIN && raw_num <= INT32_MAX) {
          int32_t num = raw_num;
          return token(num, line_number, char_index);
        } else {
          throw syntax_error("integer value too large for type 'int' (-2^31 to 2^31-1).", line_number, char_index, word.length());
        }
      } else {
        return token(identifier{std::move(word)}, line_number, char_index);
      }
    }

    token fetch_operator(push_back_stream& stream) {
      size_t line_number = stream.line_number();
      size_t char_index = stream.char_index();

      if (std::optional<reserved_token> t = get_operator(stream)) {
        return token(*t, line_number, char_index);
      } else {
        std::string unexpected;
        size_t err_line_number = stream.line_number();
        size_t err_char_index = stream.char_index();
        for (int c = stream(); get_char_type(c) == char_type::other; c = stream()) {
          unexpected.push_back(char(c));
        }
        throw unexpected_error(unexpected, err_line_number, err_char_index, false);
      }
    }

    token fetch_str(push_back_stream& stream) {
      size_t line_number = stream.line_number();
      size_t char_index = stream.char_index();

      std::string str;
      bool escaped = false;
      int c = stream();
      for (; get_char_type(c) != char_type::eof; c = stream()) {
        if (c == '\\' && !escaped) {
          escaped = true;
        } else if (escaped) {
          switch (c) {
            case 't':
              str.push_back('\t');
              break;
            case 'n':
              str.push_back('\n');
              break;
            case 'r':
              str.push_back('\r');
              break;
            case '0':
              str.push_back('\0');
              break;
            default:
              str.push_back(c);
              break;
          }
          escaped = false;
        } else {
          switch (c) {
            case '\t':
            case '\n':
            case '\r':
              stream.push_back(c);
              throw syntax_error("string was never closed.", line_number, char_index - 1, stream.char_index() - char_index);
            case '"':
              return token(std::move(str), line_number, char_index);
            default:
              str.push_back(c);
          }
        }
      }
      stream.push_back(c);
      throw syntax_error("string was never closed.", line_number, char_index - 1, stream.char_index() - char_index);
    }

    void skip_line_comment(push_back_stream& stream) {
      int c;
      do {
        c = stream();
      } while (c != '\n' && get_char_type(c) != char_type::eof);

      if (c != '\n') {
        stream.push_back(c);
      }
    }

    void skip_block_comment(push_back_stream& stream) {
      size_t line_number = stream.line_number();
      size_t char_index = stream.char_index();

      int c;
      bool closing = false;
      do {
        c = stream();
        if (closing && c == '/') {
          return;
        }
        closing = c == '*';
      } while (get_char_type(c) != char_type::eof);

      stream.push_back(c);
      throw syntax_error("block comment was never closed.", line_number, char_index - 1, 2);
    }

    token tokenize(push_back_stream& stream) {
      while (true) {
        size_t line_number = stream.line_number();
        size_t char_index = stream.char_index();
        int c = stream();
        switch (get_char_type(c)) {
          case char_type::eof:
            return {eof(), line_number, char_index};
          case char_type::whitespace:
            continue;
          case char_type::alpha:
          case char_type::numeric:
            stream.push_back(c);
            return fetch_word(stream);
          case char_type::other:
            switch (c) {
              case '"':
                return fetch_str(stream);
              case '/': {
                char c1 = stream();
                switch (c1) {
                  case '/':
                    skip_line_comment(stream);
                    continue;
                  case '*':
                    skip_block_comment(stream);
                    continue;
                  default:
                    stream.push_back(c1);
                }
              }
              default:
                stream.push_back(c);
                return fetch_operator(stream);
            }
            break;
        }
      }
    }
  }

  tokens_iterator::tokens_iterator(push_back_stream& stream):
  _stream(stream), _current(eof(), 0, 0) {
    ++(*this);
  }

  tokens_iterator& tokens_iterator::operator++() {
    _current = tokenize(_stream);
    return *this;
  }

  const token& tokens_iterator::operator*() const {
    return _current;
  }

  const token* tokens_iterator::operator->() const {
    return &_current;
  }

  tokens_iterator::operator bool() const {
    return !_current.is_eof();
  }

  void tokens_iterator::move_back(token t) {
    std::string current = std::to_string(_current);
    for (int i = current.size() - 1; i >= 0; i--) {
      _stream.push_back(current.at(i));
    }
    _current = t;
  }
}