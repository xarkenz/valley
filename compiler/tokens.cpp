#include "tokens.hpp"


namespace valley {

  namespace {
    
    const Lookup<std::string_view, ReservedToken> operatorTokenMap {
      {"&",   ReservedToken::AMPERSAND},
      {"<",   ReservedToken::ANGLE_L},
      {">",   ReservedToken::ANGLE_R},
      {"*",   ReservedToken::ASTERISK},
      {"@",   ReservedToken::AT},
      {"\\",  ReservedToken::BACKSLASH},
      {"|",   ReservedToken::BAR},
      {"^",   ReservedToken::CARET},
      {":",   ReservedToken::COLON},
      {",",   ReservedToken::COMMA},
      {"{",   ReservedToken::CURLY_L},
      {"}",   ReservedToken::CURLY_R},
      {"$",   ReservedToken::DOLLAR},
      {"=",   ReservedToken::EQUAL},
      {"!",   ReservedToken::EXCLAMATION},
      {"#",   ReservedToken::HASH},
      {"-",   ReservedToken::HYPHEN},
      {".",   ReservedToken::PERIOD},
      {"%",   ReservedToken::PERCENT},
      {"+",   ReservedToken::PLUS},
      {"?",   ReservedToken::QUESTION},
      {"(",   ReservedToken::ROUND_L},
      {")",   ReservedToken::ROUND_R},
      {";",   ReservedToken::SEMICOLON},
      {"/",   ReservedToken::SLASH},
      {"[",   ReservedToken::SQUARE_L},
      {"]",   ReservedToken::SQUARE_R},
      {"~",   ReservedToken::TILDE},

      {"&&",  ReservedToken::D_AMPERSAND},
      {"<<",  ReservedToken::D_ANGLE_L},
      {">>",  ReservedToken::D_ANGLE_R},
      {"**",  ReservedToken::D_ASTERISK},
      {"||",  ReservedToken::D_BAR},
      {"^^",  ReservedToken::D_CARET},
      {"==",  ReservedToken::D_EQUAL},
      {"--",  ReservedToken::D_HYPHEN},
      {"++",  ReservedToken::D_PLUS},

      {"&=",  ReservedToken::AMPERSAND_EQUAL},
      {"<=",  ReservedToken::ANGLE_L_EQUAL},
      {">=",  ReservedToken::ANGLE_R_EQUAL},
      {"*=",  ReservedToken::ASTERISK_EQUAL},
      {"|=",  ReservedToken::BAR_EQUAL},
      {"^=",  ReservedToken::CARET_EQUAL},
      {"!=",  ReservedToken::EXCLAMATION_EQUAL},
      {"-=",  ReservedToken::HYPHEN_EQUAL},
      {"%=",  ReservedToken::PERCENT_EQUAL},
      {"+=",  ReservedToken::PLUS_EQUAL},
      {"/=",  ReservedToken::SLASH_EQUAL},

      {"<<=", ReservedToken::D_ANGLE_L_EQUAL},
      {">>=", ReservedToken::D_ANGLE_R_EQUAL},
      {"**=", ReservedToken::D_ASTERISK_EQUAL},

      {"->",  ReservedToken::ARROW_R},
      {"...", ReservedToken::ELLIPSIS},
    };

    const Lookup<std::string_view, ReservedToken> keywordTokenMap {
      {"break",    ReservedToken::KW_BREAK},
      {"case",     ReservedToken::KW_CASE},
      {"catch",    ReservedToken::KW_CATCH},
      {"continue", ReservedToken::KW_CONTINUE},
      {"default",  ReservedToken::KW_DEFAULT},
      {"do",       ReservedToken::KW_DO},
      {"elif",     ReservedToken::KW_ELIF},
      {"else",     ReservedToken::KW_ELSE},
      {"final",    ReservedToken::KW_FINAL},
      {"finally",  ReservedToken::KW_FINALLY},
      {"for",      ReservedToken::KW_FOR},
      {"if",       ReservedToken::KW_IF},
      {"import",   ReservedToken::KW_IMPORT},
      {"return",   ReservedToken::KW_RETURN},
      {"static",   ReservedToken::KW_STATIC},
      {"switch",   ReservedToken::KW_SWITCH},
      {"try",      ReservedToken::KW_TRY},
      {"while",    ReservedToken::KW_WHILE},

      {"any",      ReservedToken::TYPE_ANY},
      {"bool",     ReservedToken::TYPE_BOOL},
      {"byte",     ReservedToken::TYPE_BYTE},
      {"char",     ReservedToken::TYPE_CHAR},
      {"class",    ReservedToken::TYPE_CLASS},
      {"double",   ReservedToken::TYPE_DOUBLE},
      {"float",    ReservedToken::TYPE_FLOAT},
      {"func",     ReservedToken::TYPE_FUNC},
      {"int",      ReservedToken::TYPE_INT},
      {"long",     ReservedToken::TYPE_LONG},
      {"short",    ReservedToken::TYPE_SHORT},
      {"str",      ReservedToken::TYPE_STR},
      {"void",     ReservedToken::TYPE_VOID},
    };

    const Lookup<ReservedToken, std::string_view> tokenStringMap = (
      [](){
        std::vector<std::pair<ReservedToken, std::string_view>> container;
        container.reserve(operatorTokenMap.size() + keywordTokenMap.size());
        for (const auto& p : operatorTokenMap) {
          container.emplace_back(p.second, p.first);
        }
        for (const auto& p : keywordTokenMap) {
          container.emplace_back(p.second, p.first);
        }
        return Lookup<ReservedToken, std::string_view>(std::move(container));
      }
    )();

    class MaximalMunchComparator {
    private:
      size_t _idx;
      
    public:
      MaximalMunchComparator(size_t idx):
        _idx(idx)
      {
      }

      bool operator()(char l, char r) const {
        return l < r;
      }

      bool operator()(std::pair<std::string_view, ReservedToken> l, char r) const {
        return l.first.size() <= _idx || l.first[_idx] < r;
      }

      bool operator()(char l, std::pair<std::string_view, ReservedToken> r) const {
        return r.first.size() > _idx && l < r.first[_idx];
      }

      bool operator()(std::pair<std::string_view, ReservedToken> l, std::pair<std::string_view, ReservedToken> r) const {
        return r.first.size() > _idx && (l.first.size() < _idx || l.first[_idx] < r.first[_idx]);
      }
    };

  }

  std::optional<ReservedToken> getKeyword(std::string_view word) {
    auto it = keywordTokenMap.find(word);
    return it == keywordTokenMap.end() ? std::nullopt : std::make_optional(it->second);
  }

  std::optional<ReservedToken> getOperator(PushBackStream& stream) {
    auto candidates = std::make_pair(operatorTokenMap.begin(), operatorTokenMap.end());

    std::optional<ReservedToken> ret;
    size_t match_size = 0;
    std::stack<int> chars;

    for (size_t idx = 0; candidates.first != candidates.second; ++idx) {
      chars.push(stream());
      
      candidates = std::equal_range(candidates.first, candidates.second, char(chars.top()), MaximalMunchComparator(idx));

      if (candidates.first != candidates.second && candidates.first->first.size() == idx + 1) {
        match_size = idx + 1;
        ret = candidates.first->second;
      }
    }

    while (chars.size() > match_size) {
      stream.pushBack(chars.top());
      chars.pop();
    }

    return ret;
  }

  bool operator==(const Identifier& id1, const Identifier& id2) {
    return id1.name == id2.name;
  }

  bool operator!=(const Identifier& id1, const Identifier& id2) {
    return id1.name != id2.name;
  }

  bool operator==(const Eof&, const Eof&) {
    return true;
  }

  bool operator!=(const Eof&, const Eof&) {
    return false;
  }

  bool operator==(const VoidValue&, const VoidValue&) {
    return true;
  }

  bool operator!=(const VoidValue&, VoidValue&) {
    return false;
  }

  Token::Token(TokenValue value, size_t lineNumber, size_t charIndex):
	  _value(std::move(value)),
    _line_number(lineNumber),
    _char_index(charIndex)
  {
	}

  bool Token::isEof() const {
		return std::holds_alternative<Eof>(_value);
	}
	
	bool Token::isReservedToken() const {
		return std::holds_alternative<ReservedToken>(_value);
	}
	
	bool Token::isIdentifier() const {
		return std::holds_alternative<Identifier>(_value);
	}

  bool Token::isNull() const {
    return std::holds_alternative<VoidValue>(_value);
  }

  bool Token::isByte() const {
		return std::holds_alternative<int8_t>(_value);
	}

  bool Token::isShort() const {
		return std::holds_alternative<int16_t>(_value);
	}

  bool Token::isInt() const {
		return std::holds_alternative<int32_t>(_value);
	}

  bool Token::isLong() const {
		return std::holds_alternative<int64_t>(_value);
	}

  bool Token::isFloat() const {
		return std::holds_alternative<float>(_value);
	}
	
	bool Token::isDouble() const {
		return std::holds_alternative<double>(_value);
	}

  bool Token::isBool() const {
		return std::holds_alternative<bool>(_value);
	}

  bool Token::isChar() const {
		return std::holds_alternative<char>(_value);
	}
	
	bool Token::isStr() const {
		return std::holds_alternative<std::string>(_value);
	}

  bool Token::isLiteral() const {
    return isNull() || isByte() || isShort() || isInt() || isLong() || isFloat() || isDouble() || isBool() || isChar() || isStr();
  }

  bool Token::isIntegral() const {
    return isByte() || isShort() || isInt() || isLong() || isBool() || isChar();
  }

  bool Token::isNumeric() const {
    return isByte() || isShort() || isInt() || isLong() || isFloat() || isDouble() || isBool() || isChar();
  }

  const TokenValue& Token::getValue() const {
    return _value;
  }
	
	ReservedToken Token::getReservedToken() const {
		return std::get<ReservedToken>(_value);
	}
	
	const Identifier& Token::getIdentifier() const {
		return std::get<Identifier>(_value);
	}

  int8_t Token::getByte() const {
		return std::get<int8_t>(_value);
	}

  int16_t Token::getShort() const {
		return std::get<int16_t>(_value);
	}

  int32_t Token::getInt() const {
		return std::get<int32_t>(_value);
	}

  int64_t Token::getLong() const {
		return std::get<int64_t>(_value);
	}

  float Token::getFloat() const {
		return std::get<float>(_value);
	}
	
	double Token::getDouble() const {
		return std::get<double>(_value);
	}

  bool Token::getBool() const {
		return std::get<bool>(_value);
	}

  char Token::getChar() const {
		return std::get<char>(_value);
	}
	
	const std::string& Token::getStr() const {
		return std::get<std::string>(_value);
	}
	
	size_t Token::lineNumber() const {
		return _line_number;
	}

	size_t Token::charIndex() const {
		return _char_index;
	}

  bool Token::hasValue(TokenValue value) const {
    return _value == value;
  }

  std::string Token::toString() const {
    return std::visit(Overloaded{
      [](VoidValue) {
        return std::string("null");
      },
      [](ReservedToken rt) {
        return reservedTokenRepr(rt);
      },
      [](int8_t num) {
        return std::to_string(num);
      },
      [](int16_t num) {
        return std::to_string(num);
      },
      [](int32_t num) {
        return std::to_string(num);
      },
      [](int64_t num) {
        return std::to_string(num);
      },
      [](float num) {
        return std::to_string(num);
      },
      [](double num) {
        return std::to_string(num);
      },
      [](bool b) {
        return b ? std::string("true") : std::string("false");
      },
      [](char c) {
        std::string s = "'";
        s += c;
        return s + "'";
      },
      [](const std::string& str) {
        std::string s = "\"";
        s += str;
        s += "\"";
        return s;
      },
      [](const Identifier& id) {
        return id.name;
      },
      [](Eof) {
        return std::string("");
      }
    }, getValue());
  }

  std::string reservedTokenRepr(ReservedToken t) {
    return std::string(tokenStringMap.find(t)->second);
  }
}