#include "errors.hpp"


namespace valley {

  Error::Error(std::string name, std::string message, size_t lineNumber, size_t charIndex, size_t length) noexcept:
    _name(std::move(name)),
    _message(std::move(message)),
    _line_number(lineNumber),
    _char_index(charIndex),
    _length(length)
  {
  }

  const char* Error::name() const noexcept {
    return _name.c_str();
  }

  const char* Error::what() const noexcept {
    return _message.c_str();
  }

  size_t Error::lineNumber() const noexcept {
    return _line_number;
  }

  size_t Error::charIndex() const noexcept {
    return _char_index;
  }

  size_t Error::length() const noexcept {
    return _length;
  }

  void Error::format(CharGetter source, std::ostream& output) {
    output << _name << " (line " << (_line_number + 1) << "): " << _message << std::endl;

    size_t index = 0;

    for (size_t lineNumber = 0; lineNumber < _line_number; ++index) {
      int c = source();
      if (c < 0) {
        return;
      } else if (c == '\n') {
        ++lineNumber;
      }
    }

    size_t indexInLine = _char_index - index;

    std::string line;
    for (size_t idx = 0;; ++idx) {
      int c = source();
      if (c < 0 || c == '\n' || c == '\r') {
        break;
      }
      line += char(c);
    }

    output << "  " << line << std::endl << "  ";

    for (size_t idx = 0; idx < indexInLine; ++idx) {
      output << " ";
    }

    if (_length == 0) {
      output << "^" << std::endl;
    } else {
      for (size_t idx = 0; idx < _length; ++idx) {
        output << "~";
      }
      output << std::endl;
    }
  }

  Error SyntaxError(std::string_view message, size_t lineNumber, size_t charIndex, size_t length) {
    return Error("SyntaxError", std::string(message), lineNumber, charIndex, length);
  }

  Error SemanticError(std::string_view message, size_t lineNumber, size_t charIndex, size_t length) {
    return Error("SemanticError", std::string(message), lineNumber, charIndex, length);
  }

  Error CompileError(std::string_view message, size_t lineNumber, size_t charIndex, size_t length) {
    return Error("CompileError", std::string(message), lineNumber, charIndex, length);
  }

  Error TypeError(std::string_view source, std::string_view destination, size_t lineNumber, size_t charIndex, size_t length) {
    std::string message("cannot convert '");
    message += source;
    message += "' object to '";
    message += destination;
    return Error("TypeError", message + "'.", lineNumber, charIndex, length);
  }

  Error RuntimeError(std::string_view message, size_t lineNumber, size_t charIndex) {
    return Error("RuntimeError", std::string(message), lineNumber, charIndex, 0);
  }

  Error SyntaxError_unexpected(std::string_view unexpected, size_t lineNumber, size_t charIndex, bool point_at) {
    std::string message("encountered unexpected '");
    message += unexpected;
    message += "' while parsing.";
    size_t length = 0;
    if (!point_at)
      length = unexpected.length();
    return SyntaxError(message, lineNumber, charIndex, length);
  }
  
}