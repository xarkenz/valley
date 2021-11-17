#include "tokenizer.hpp"

#include "errors.hpp"


namespace valley {

  namespace {

    enum struct CharType {
      END,
      WHITESPACE,
      ALPHA,
      NUMERIC,
      OTHER,
    };

    CharType getCharType(int c) {
      if (c < 0)
        return CharType::END;

      if (std::isspace(c))
        return CharType::WHITESPACE;
      
      if (std::isalpha(c) || c == '_')
        return CharType::ALPHA;
      
      if (std::isdigit(c)) 
        return CharType::NUMERIC;
      
      return CharType::OTHER;
    }

    Token fetchWord(PushBackStream& stream) {
      size_t lineNumber = stream.lineNumber();
      size_t charIndex = stream.charIndex();

      std::string word;
      int c = stream();
      bool isNumber = std::isdigit(c) || c == '.';
      bool isDouble = false;

      do {
        word.push_back(char(c));
        c = stream();
        if (isNumber && c == '.') {
          if (!isDouble)
            isDouble = true;
          else
            throw SyntaxError_unexpected(std::string(1, char(c)), stream.charIndex(), stream.lineNumber(), true);
        }
      } while (getCharType(c) == CharType::ALPHA || getCharType(c) == CharType::NUMERIC || (isNumber && c == '.'));

      stream.pushBack(c);

      if (std::optional<ReservedToken> t = getKeyword(word)) {
        if (*t == ReservedToken::KW_ELIF) {
          // A little hacky, interprets 'elif' as 'else' and artificially adds 'if' to the stream to be the next token
          stream.pushBack('f');
          stream.pushBack('i');
          return Token(ReservedToken::KW_ELSE, lineNumber, charIndex);
        }
        return Token(*t, lineNumber, charIndex);
      } else if (isDouble) {
        char* endPtr;
        double rawNum = strtod(word.c_str(), &endPtr);
        if (*endPtr) {
          if (*endPtr == 'F' || *endPtr == 'f') {
            float num = rawNum;
            return Token(num, lineNumber, charIndex);
          } else {
            // Pointer arithmetic...
            size_t remaining = word.length() - (endPtr - word.c_str());
            throw SyntaxError_unexpected(std::string(1, char(*endPtr)), stream.lineNumber(), stream.charIndex() - remaining, false);
          }
        }
        return Token(rawNum, lineNumber, charIndex);
      } else if (isNumber) {
        char* endPtr;
        long int rawNum = strtol(word.c_str(), &endPtr, 10);
        if (*endPtr) {
          if (*endPtr == 'B' || *endPtr == 'b') {
            if (rawNum >= INT8_MIN && rawNum <= INT8_MAX) {
              int8_t num = rawNum;
              return Token(num, lineNumber, charIndex);
            } else {
              throw SyntaxError("integer value out of range for type 'byte' (-2^7 to 2^7-1).", lineNumber, charIndex, word.length());
            }
          } else if (*endPtr == 'S' || *endPtr == 's') {
            if (rawNum >= INT16_MIN && rawNum <= INT16_MAX) {
              int16_t num = rawNum;
              return Token(num, lineNumber, charIndex);
            } else {
              throw SyntaxError("integer value too large for type 'short' (-2^15 to 2^15-1).", lineNumber, charIndex, word.length());
            }
          } else if (*endPtr == 'L' || *endPtr == 'l') {
            if (rawNum >= INT64_MIN && rawNum <= INT64_MAX) {
              int64_t num = rawNum;
              return Token(num, lineNumber, charIndex);
            } else {
              throw SyntaxError("integer value too large for type 'long' (-2^63 to 2^63-1).", lineNumber, charIndex, word.length());
            }
          } else if (*endPtr != 'I' && *endPtr != 'i') {
            size_t remaining = word.length() - (endPtr - word.c_str());
            throw SyntaxError_unexpected(std::string(1, char(*endPtr)), stream.lineNumber(), stream.charIndex() - remaining, word.length());
          }
        }
        if (rawNum >= INT32_MIN && rawNum <= INT32_MAX) {
          int32_t num = rawNum;
          return Token(num, lineNumber, charIndex);
        } else {
          throw SyntaxError("integer value too large for type 'int' (-2^31 to 2^31-1).", lineNumber, charIndex, word.length());
        }
      } else {
        return Token(Identifier{std::move(word)}, lineNumber, charIndex);
      }
    }

    Token fetchOperator(PushBackStream& stream) {
      size_t lineNumber = stream.lineNumber();
      size_t charIndex = stream.charIndex();

      if (std::optional<ReservedToken> t = getOperator(stream)) {
        return Token(*t, lineNumber, charIndex);
      } else {
        std::string unexpected;
        size_t err_lineNumber = stream.lineNumber();
        size_t err_charIndex = stream.charIndex();
        for (int c = stream(); getCharType(c) == CharType::OTHER; c = stream()) {
          unexpected.push_back(char(c));
        }
        throw SyntaxError_unexpected(unexpected, err_lineNumber, err_charIndex, false);
      }
    }

    Token fetchString(PushBackStream& stream) {
      size_t lineNumber = stream.lineNumber();
      size_t charIndex = stream.charIndex();

      std::string str;
      bool escaped = false;
      int c = stream();
      for (; getCharType(c) != CharType::END; c = stream()) {
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
              stream.pushBack(c);
              throw SyntaxError("could not find a matching '\"'.", lineNumber, charIndex - 1, stream.charIndex() - charIndex);
            case '"':
              return Token(std::move(str), lineNumber, charIndex);
            default:
              str.push_back(c);
          }
        }
      }
      stream.pushBack(c);
      throw SyntaxError("could not find a matching '\"'.", lineNumber, charIndex - 1, stream.charIndex() - charIndex);
    }

    void skipLineComment(PushBackStream& stream) {
      int c;
      do {
        c = stream();
      } while (c != '\n' && getCharType(c) != CharType::END);
      if (c != '\n') {
        stream.pushBack(c);
      }
    }

    void skipBlockComment(PushBackStream& stream) {
      size_t lineNumber = stream.lineNumber();
      size_t charIndex = stream.charIndex();

      int c;
      bool closing = false;
      do {
        c = stream();
        if (closing && c == '/') {
          return;
        }
        closing = c == '*';
      } while (getCharType(c) != CharType::END);

      stream.pushBack(c);
      throw SyntaxError("could not find a matching '*/'.", lineNumber, charIndex - 1, 2);
    }

    Token tokenize(PushBackStream& stream) {
      while (true) {
        size_t lineNumber = stream.lineNumber();
        size_t charIndex = stream.charIndex();
        int c = stream();
        switch (getCharType(c)) {
          case CharType::END:
            return {Eof(), lineNumber, charIndex};

          case CharType::WHITESPACE:
            continue;
            
          case CharType::ALPHA:
          case CharType::NUMERIC:
            stream.pushBack(c);
            return fetchWord(stream);

          case CharType::OTHER:
            switch (c) {
              case '"':
                return fetchString(stream);
              case '/': {
                char c1 = stream();
                switch (c1) {
                  case '/':
                    skipLineComment(stream);
                    continue;
                  case '*':
                    skipBlockComment(stream);
                    continue;
                  default:
                    stream.pushBack(c1);
                }
              }
              default:
                stream.pushBack(c);
                return fetchOperator(stream);
            }
            break;
        }
      }
    }
    
  }

  TokenIterator::TokenIterator(PushBackStream& stream):
    _stream(stream),
    _current(Eof(), 0, 0)
  {
    ++(*this);
  }

  TokenIterator& TokenIterator::operator++() {
    _current = tokenize(_stream);
    return *this;
  }

  const Token& TokenIterator::operator*() const {
    return _current;
  }

  const Token* TokenIterator::operator->() const {
    return &_current;
  }

  TokenIterator::operator bool() const {
    return !_current.isEof();
  }

  void TokenIterator::stepBack(Token t) {
    std::string current = _current.toString();
    for (int i = current.size() - 1; i >= 0; i--) {
      _stream.pushBack(current.at(i));
    }
    _current = t;
  }

}