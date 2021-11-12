#include "errors.hpp"

namespace valley {
  error::error(std::string name, std::string message, size_t line_number, size_t char_index, size_t length) noexcept:
  _name(std::move(name)), _message(std::move(message)), _line_number(line_number), _char_index(char_index), _length(length) {
  }

  const char* error::name() const noexcept {
    return _name.c_str();
  }

  const char* error::what() const noexcept {
    return _message.c_str();
  }

  size_t error::line_number() const noexcept {
    return _line_number;
  }

  size_t error::char_index() const noexcept {
    return _char_index;
  }

  size_t error::length() const noexcept {
    return _length;
  }

  void error::format(get_character source, std::ostream& output) {
    output << _name << " (line " << (_line_number + 1) << "): " << _message << std::endl;

    size_t index = 0;

    for (size_t line_number = 0; line_number < _line_number; ++index) {
      int c = source();
      if (c < 0) {
        return;
      } else if (c == '\n') {
        ++line_number;
      }
    }

    size_t index_in_line = _char_index - index;

    std::string line;
    for (size_t idx = 0;; ++idx) {
      int c = source();
      if (c < 0 || c == '\n' || c == '\r') {
        break;
      }
      line += char(c);
    }

    output << "  " << line << std::endl << "  ";

    for (size_t idx = 0; idx < index_in_line; ++idx) {
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

  error parsing_error(std::string_view message, size_t line_number, size_t char_index, size_t length) {
    return error("Parsing error", std::string(message), line_number, char_index, length);
  }

  error syntax_error(std::string_view message, size_t line_number, size_t char_index, size_t length) {
    return error("Syntax error", std::string(message), line_number, char_index, length);
  }

  error semantic_error(std::string_view message, size_t line_number, size_t char_index, size_t length) {
    return error("Semantic error", std::string(message), line_number, char_index, length);
  }

  error compiler_error(std::string_view message, size_t line_number, size_t char_index, size_t length) {
    return error("Compiler error", std::string(message), line_number, char_index, length);
  }

  error runtime_error(std::string_view message, size_t line_number, size_t char_index) {
    return error("Runtime error", std::string(message), line_number, char_index, 0);
  }

  error unexpected_error(std::string_view unexpected, size_t line_number, size_t char_index, bool point_at) {
    std::string message("encountered unexpected '");
    message += unexpected;
    message += "' while parsing.";
    size_t length = 0;
    if (!point_at) {
      length = unexpected.length();
    }
    return parsing_error(message, line_number, char_index, length);
  }

  error unexpected_syntax_error(std::string_view unexpected, size_t line_number, size_t char_index, bool point_at) {
    std::string message("encountered unexpected '");
    message += unexpected;
    message += "' while parsing.";
    size_t length = 0;
    if (!point_at) {
      length = unexpected.length();
    }
    return syntax_error(message, line_number, char_index, length);
  }

  error undeclared_error(std::string_view undeclared, size_t line_number, size_t char_index, size_t length) {
    std::string message("identifier '");
    message += undeclared;
    message += "' has not been declared.";
    return semantic_error(message, line_number, char_index, length);
  }

  error wrong_type_error(std::string_view source, std::string_view destination, size_t line_number, size_t char_index, size_t length) {
    std::string message("cannot convert '");
    message += source;
    message += "' object to '";
    message += destination;
    message += "'.";
    return semantic_error(message, line_number, char_index, length);
  }

  error not_lvalue_error(std::string_view type_name, size_t line_number, size_t char_index, size_t length) {
    std::string message("this '");
    message += type_name;
    message += "' object cannot be assigned to.";
    return semantic_error(message, line_number, char_index, length);
  }
}