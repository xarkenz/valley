#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <exception>
#include <functional>
#include <ostream>
#include <string>
#include <string_view>

namespace valley {
  using get_character = std::function<int()>;

  class error: public std::exception {
    private:
      std::string _name;
      std::string _message;
      size_t _line_number;
      size_t _char_index;
      size_t _length;

    public:
      error(std::string name, std::string message, size_t line_number, size_t char_index, size_t length) noexcept;

      const char* name() const noexcept;
      const char* what() const noexcept override;
      size_t line_number() const noexcept;
      size_t char_index() const noexcept;
      size_t length() const noexcept;
      void format(get_character source, std::ostream& output);
  };

  error parsing_error(std::string_view message, size_t line_number, size_t char_index, size_t length);
  error syntax_error(std::string_view message, size_t line_number, size_t char_index, size_t length);
  error semantic_error(std::string_view message, size_t line_number, size_t char_index, size_t length);
  error compiler_error(std::string_view message, size_t line_number, size_t char_index, size_t length);
  error runtime_error(std::string_view message, size_t line_number, size_t char_index);

  error unexpected_error(std::string_view unexpected, size_t line_number, size_t char_index, bool point_at);
  error unexpected_syntax_error(std::string_view unexpected, size_t line_number, size_t char_index, bool point_at);
  error undeclared_error(std::string_view undeclared, size_t line_number, size_t char_index, size_t length);
  error wrong_type_error(std::string_view source, std::string_view destination, size_t line_number, size_t char_index, size_t length);
  error not_lvalue_error(std::string_view type_name, size_t line_number, size_t char_index, size_t length);
}

#endif