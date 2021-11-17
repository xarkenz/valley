#pragma once

#include <exception>
#include <functional>
#include <ostream>
#include <string>
#include <string_view>


namespace valley {

  using CharGetter = std::function<int()>;

  class Error: public std::exception {
  private:
    std::string _name;
    std::string _message;
    size_t _line_number;
    size_t _char_index;
    size_t _length;

  public:
    Error(std::string name, std::string message, size_t lineNumber, size_t charIndex, size_t length) noexcept;

    const char* name() const noexcept;
    const char* what() const noexcept override;
    size_t lineNumber() const noexcept;
    size_t charIndex() const noexcept;
    size_t length() const noexcept;
    void format(CharGetter source, std::ostream& output);
  };

  Error SyntaxError(std::string_view message, size_t line_number, size_t char_index, size_t length);
  Error SemanticError(std::string_view message, size_t line_number, size_t char_index, size_t length);
  Error CompileError(std::string_view message, size_t line_number, size_t char_index, size_t length);
  Error TypeError(std::string_view source, std::string_view destination, size_t line_number, size_t char_index, size_t length);
  Error RuntimeError(std::string_view message, size_t line_number, size_t char_index);

  Error SyntaxError_unexpected(std::string_view unexpected, size_t line_number, size_t char_index, bool point_at);

}