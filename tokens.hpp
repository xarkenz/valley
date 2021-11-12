#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <cstdint>
#include <optional>
#include <ostream>
#include <string_view>
#include <variant>

#include "push_back_stream.hpp"
#include "types.hpp"

namespace valley {
  enum struct reserved_token {
    ampr,
    atsym,
    bslsh,
    caret,
    colon,
    comma,
    dollr,
    equal,
    exclm,
    hash,
    minus,
    plus,
    point,
    prcnt,
    qmark,
    semic,
    slash,
    star,
    tilde,
    vbar,

    langle,
    rangle,
    lbrace,
    rbrace,
    lbrckt,
    rbrckt,
    lparen,
    rparen,

    ampr_eq,
    caret_eq,
    exclm_eq,
    minus_eq,
    plus_eq,
    prcnt_eq,
    slash_eq,
    star_eq,
    vbar_eq,

    langle_eq,
    rangle_eq,

    d_ampr,
    d_caret,
    d_equal,
    d_minus,
    d_plus,
    d_star,
    d_vbar,

    d_langle,
    d_rangle,

    d_star_eq,

    d_langle_eq,
    d_rangle_eq,

    ellipsis,
    arrow,

    kw_import,
    kw_package,

    kw_if,
    kw_elif,
    kw_else,

    kw_switch,
    kw_case,
    kw_default,

    kw_for,
    kw_while,
    kw_do,

    kw_break,
    kw_continue,
    kw_return,

    kw_try,
    kw_catch,
    kw_finally,

    kw_final,
    kw_static,

    typekw_any,
    typekw_bool,
    typekw_byte,
    typekw_char,
    typekw_class,
    typekw_double,
    typekw_float,
    typekw_func,
    typekw_int,
    typekw_long,
    typekw_short,
    typekw_str,
    typekw_void,
  };

  std::optional<reserved_token> get_keyword(std::string_view word);
  std::optional<reserved_token> get_operator(push_back_stream& stream);

  struct identifier {
    std::string name;
  };

  bool operator==(const identifier& id1, const identifier& id2);
  bool operator!=(const identifier& id1, const identifier& id2);

  struct eof {
  };

  bool operator==(const eof&, const eof&);
  bool operator!=(const eof&, const eof&);

  struct void_value {
  };

  bool operator==(const void_value&, const void_value&);
  bool operator!=(const void_value&, const void_value&);

  using token_value = std::variant<
    eof,
    reserved_token,
    identifier,
    void_value, // void
    int8_t, // byte
    int16_t, // short
    int32_t, // int
    int64_t, // long
    float, // float
    double, // double
    bool, // bool
    char, // char
    std::string // str
  >;

  class token {
    private:
      token_value _value;
      size_t _line_number;
      size_t _char_index;
    
    public:
      token(token_value value, size_t line_number, size_t char_index);

      bool is_eof() const;
      bool is_reserved_token() const;
      bool is_identifier() const;
      bool is_null() const;
      bool is_byte() const;
      bool is_short() const;
      bool is_int() const;
      bool is_long() const;
      bool is_float() const;
      bool is_double() const;
      bool is_bool() const;
      bool is_char() const;
      bool is_str() const;

      bool is_literal() const;
      bool is_integer() const;
      bool is_numeric() const;

      const token_value& get_value() const;

      reserved_token get_reserved_token() const;
      const identifier& get_identifier() const;
      int8_t get_byte() const;
      int16_t get_short() const;
      int32_t get_int() const;
      int64_t get_long() const;
      float get_float() const;
      double get_double() const;
      bool get_bool() const;
      char get_char() const;
      const std::string& get_str() const;

      size_t get_line_number() const;
      size_t get_char_index() const;

      bool has_value(token_value value) const;
  };
}

namespace std {
  string to_string(valley::reserved_token t);
  string to_string(const valley::token& t);
}

#endif