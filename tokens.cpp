#include "tokens.hpp"

#include <string_view>
#include <iostream>

#include "lookup.hpp"
#include "helpers.hpp"

namespace valley {
  namespace {
    const lookup<std::string_view, reserved_token> operator_token_map {
      {"&", reserved_token::ampr},
      {"@", reserved_token::atsym},
      {"\\", reserved_token::bslsh},
      {"^", reserved_token::caret},
      {":", reserved_token::colon},
      {",", reserved_token::comma},
      {"$", reserved_token::dollr},
      {"=", reserved_token::equal},
      {"!", reserved_token::exclm},
      {"#", reserved_token::hash},
      {"-", reserved_token::minus},
      {"+", reserved_token::plus},
      {".", reserved_token::point},
      {"%", reserved_token::prcnt},
      {"?", reserved_token::qmark},
      {";", reserved_token::semic},
      {"/", reserved_token::slash},
      {"*", reserved_token::star},
      {"~", reserved_token::tilde},
      {"|", reserved_token::vbar},

      {"<", reserved_token::langle},
      {">", reserved_token::rangle},
      {"{", reserved_token::lbrace},
      {"}", reserved_token::rbrace},
      {"[", reserved_token::lbrckt},
      {"]", reserved_token::rbrckt},
      {"(", reserved_token::lparen},
      {")", reserved_token::rparen},

      {"&=", reserved_token::ampr_eq},
      {"^=", reserved_token::caret_eq},
      {"!=", reserved_token::exclm_eq},
      {"-=", reserved_token::minus_eq},
      {"+=", reserved_token::plus_eq},
      {"%=", reserved_token::prcnt_eq},
      {"/=", reserved_token::slash_eq},
      {"*=", reserved_token::star_eq},
      {"|=", reserved_token::vbar_eq},

      {"<=", reserved_token::langle_eq},
      {">=", reserved_token::rangle_eq},

      {"&&", reserved_token::d_ampr},
      {"^^", reserved_token::d_caret},
      {"==", reserved_token::d_equal},
      {"--", reserved_token::d_minus},
      {"++", reserved_token::d_plus},
      {"**", reserved_token::d_star},
      {"||", reserved_token::d_vbar},

      {"<<", reserved_token::d_langle},
      {">>", reserved_token::d_rangle},

      {"**=", reserved_token::d_star_eq},

      {"<<=", reserved_token::d_langle_eq},
      {">>=", reserved_token::d_rangle_eq},

      {"...", reserved_token::ellipsis},
      {"->", reserved_token::arrow},
    };

    const lookup<std::string_view, reserved_token> keyword_token_map {
      {"import", reserved_token::kw_import},
      {"package", reserved_token::kw_package},

      {"if", reserved_token::kw_if},
      {"elif", reserved_token::kw_elif},
      {"else", reserved_token::kw_else},

      {"switch", reserved_token::kw_switch},
      {"case", reserved_token::kw_case},
      {"default", reserved_token::kw_default},

      {"for", reserved_token::kw_for},
      {"while", reserved_token::kw_while},
      {"do", reserved_token::kw_do},
      
      {"break", reserved_token::kw_break},
      {"continue", reserved_token::kw_continue},
      {"return", reserved_token::kw_return},

      {"try", reserved_token::kw_try},
      {"catch", reserved_token::kw_catch},
      {"finally", reserved_token::kw_finally},

      {"final", reserved_token::kw_final},
      {"static", reserved_token::kw_static},

      {"any", reserved_token::typekw_any},
      {"bool", reserved_token::typekw_bool},
      {"byte", reserved_token::typekw_byte},
      {"char", reserved_token::typekw_char},
      {"class", reserved_token::typekw_class},
      {"double", reserved_token::typekw_double},
      {"float", reserved_token::typekw_float},
      {"func", reserved_token::typekw_func},
      {"int", reserved_token::typekw_int},
      {"long", reserved_token::typekw_long},
      {"short", reserved_token::typekw_short},
      {"str", reserved_token::typekw_str},
      {"void", reserved_token::typekw_void},
    };

    const lookup<reserved_token, std::string_view> token_string_map = ([](){
      std::vector<std::pair<reserved_token, std::string_view>> container;
      container.reserve(operator_token_map.size() + keyword_token_map.size());
      for (const auto& p : operator_token_map) {
        container.emplace_back(p.second, p.first);
      }
      for (const auto& p : keyword_token_map) {
        container.emplace_back(p.second, p.first);
      }
      return lookup<reserved_token, std::string_view>(std::move(container));
    })();
  }

  std::optional<reserved_token> get_keyword(std::string_view word) {
    auto it = keyword_token_map.find(word);
    return it == keyword_token_map.end() ? std::nullopt : std::make_optional(it->second);
  }

  namespace {
    class maximal_munch_comparator {
      private:
        size_t _idx;
        
      public:
        maximal_munch_comparator(size_t idx): _idx(idx) {
        }

        bool operator()(char l, char r) const {
          return l < r;
        }

        bool operator()(std::pair<std::string_view, reserved_token> l, char r) const {
          return l.first.size() <= _idx || l.first[_idx] < r;
        }

        bool operator()(char l, std::pair<std::string_view, reserved_token> r) const {
          return r.first.size() > _idx && l < r.first[_idx];
        }

        bool operator()(std::pair<std::string_view, reserved_token> l, std::pair<std::string_view, reserved_token> r) const {
          return r.first.size() > _idx && (l.first.size() < _idx || l.first[_idx] < r.first[_idx]);
        }
    };
  }

  std::optional<reserved_token> get_operator(push_back_stream& stream) {
    auto candidates = std::make_pair(operator_token_map.begin(), operator_token_map.end());

    std::optional<reserved_token> ret;
    size_t match_size = 0;
    std::stack<int> chars;

    for (size_t idx = 0; candidates.first != candidates.second; ++idx) {
      chars.push(stream());
      
      candidates = std::equal_range(candidates.first, candidates.second, char(chars.top()), maximal_munch_comparator(idx));

      if (candidates.first != candidates.second && candidates.first->first.size() == idx + 1) {
        match_size = idx + 1;
        ret = candidates.first->second;
      }
    }

    while (chars.size() > match_size) {
      stream.push_back(chars.top());
      chars.pop();
    }

    return ret;
  }

  token::token(token_value value, size_t line_number, size_t char_index):
	_value(std::move(value)), _line_number(line_number), _char_index(char_index) {
	}

  bool token::is_eof() const {
		return std::holds_alternative<eof>(_value);
	}
	
	bool token::is_reserved_token() const {
		return std::holds_alternative<reserved_token>(_value);
	}
	
	bool token::is_identifier() const {
		return std::holds_alternative<identifier>(_value);
	}

  bool token::is_null() const {
    return std::holds_alternative<void_value>(_value);
  }

  bool token::is_byte() const {
		return std::holds_alternative<int8_t>(_value);
	}

  bool token::is_short() const {
		return std::holds_alternative<int16_t>(_value);
	}

  bool token::is_int() const {
		return std::holds_alternative<int32_t>(_value);
	}

  bool token::is_long() const {
		return std::holds_alternative<int64_t>(_value);
	}

  bool token::is_float() const {
		return std::holds_alternative<float>(_value);
	}
	
	bool token::is_double() const {
		return std::holds_alternative<double>(_value);
	}

  bool token::is_bool() const {
		return std::holds_alternative<bool>(_value);
	}

  bool token::is_char() const {
		return std::holds_alternative<char>(_value);
	}
	
	bool token::is_str() const {
		return std::holds_alternative<std::string>(_value);
	}

  bool token::is_literal() const {
    return is_null() || is_byte() || is_short() || is_int() || is_long() || is_float() || is_double() || is_bool() || is_char() || is_str();
  }

  bool token::is_integer() const {
    return is_byte() || is_short() || is_int() || is_long() || is_bool() || is_char();
  }

  bool token::is_numeric() const {
    return is_byte() || is_short() || is_int() || is_long() || is_float() || is_double() || is_bool() || is_char();
  }

  const token_value& token::get_value() const {
    return _value;
  }
	
	reserved_token token::get_reserved_token() const {
		return std::get<reserved_token>(_value);
	}
	
	const identifier& token::get_identifier() const {
		return std::get<identifier>(_value);
	}

  int8_t token::get_byte() const {
		return std::get<int8_t>(_value);
	}

  int16_t token::get_short() const {
		return std::get<int16_t>(_value);
	}

  int32_t token::get_int() const {
		return std::get<int32_t>(_value);
	}

  int64_t token::get_long() const {
		return std::get<int64_t>(_value);
	}

  float token::get_float() const {
		return std::get<float>(_value);
	}
	
	double token::get_double() const {
		return std::get<double>(_value);
	}

  bool token::get_bool() const {
		return std::get<bool>(_value);
	}

  char token::get_char() const {
		return std::get<char>(_value);
	}
	
	const std::string& token::get_str() const {
		return std::get<std::string>(_value);
	}
	
	size_t token::get_line_number() const {
		return _line_number;
	}

	size_t token::get_char_index() const {
		return _char_index;
	}

  bool token::has_value(token_value value) const {
    return _value == value;
  }

  bool operator==(const identifier& id1, const identifier& id2) {
    return id1.name == id2.name;
  }

  bool operator!=(const identifier& id1, const identifier& id2) {
    return id1.name != id2.name;
  }

  bool operator==(const eof&, const eof&) {
    return true;
  }

  bool operator!=(const eof&, const eof&) {
    return false;
  }

  bool operator==(const void_value&, const void_value&) {
    return true;
  }

  bool operator!=(const void_value&, void_value&) {
    return false;
  }
}

namespace std {
  using namespace valley;

  std::string to_string(reserved_token t) {
    return std::string(token_string_map.find(t)->second);
  }

  std::string to_string(const token& t) {
    return std::visit(overloaded{
      [](void_value) {
        return std::string("null");
      },
      [](reserved_token rt) {
        return to_string(rt);
      },
      [](int8_t num) {
        return to_string(num);
      },
      [](int16_t num) {
        return to_string(num);
      },
      [](int32_t num) {
        return to_string(num);
      },
      [](int64_t num) {
        return to_string(num);
      },
      [](float num) {
        return to_string(num);
      },
      [](double num) {
        return to_string(num);
      },
      [](bool b) {
        if (b) {
          return std::string("true");
        } else {
          return std::string("false");
        }
      },
      [](char c) {
        std::string s = "'";
        s += c;
        s += "'";
        return s;
      },
      [](const std::string& str) {
        std::string s = "\"";
        s += str;
        s += "\"";
        return s;
      },
      [](const identifier& id) {
        return id.name;
      },
      [](eof) {
        return std::string("");
      }
    }, t.get_value());
  }
}