#pragma once

#include <cstdint>
#include <optional>
#include <ostream>
#include <string_view>
#include <variant>

#include "types.hpp"
#include "util.hpp"


namespace valley {

  enum struct ReservedToken {
    AMPERSAND,
    ANGLE_L,
    ANGLE_R,
    ASTERISK,
    AT,
    BACKSLASH,
    BAR,
    CARET,
    COLON,
    COMMA,
    CURLY_L,
    CURLY_R,
    DOLLAR,
    EQUAL,
    EXCLAMATION,
    HASH,
    HYPHEN,
    PERIOD,
    PERCENT,
    PLUS,
    QUESTION,
    ROUND_L,
    ROUND_R,
    SEMICOLON,
    SLASH,
    SQUARE_L,
    SQUARE_R,
    TILDE,

    D_AMPERSAND,
    D_ANGLE_L,
    D_ANGLE_R,
    D_ASTERISK,
    D_BAR,
    D_CARET,
    D_EQUAL,
    D_HYPHEN,
    D_PLUS,

    AMPERSAND_EQUAL,
    ANGLE_L_EQUAL,
    ANGLE_R_EQUAL,
    ASTERISK_EQUAL,
    BAR_EQUAL,
    CARET_EQUAL,
    EXCLAMATION_EQUAL,
    HYPHEN_EQUAL,
    PERCENT_EQUAL,
    PLUS_EQUAL,
    SLASH_EQUAL,

    D_ANGLE_L_EQUAL,
    D_ANGLE_R_EQUAL,
    D_ASTERISK_EQUAL,

    ARROW_R,
    ELLIPSIS,

    KW_BREAK,
    KW_CASE,
    KW_CATCH,
    KW_CONTINUE,
    KW_DEFAULT,
    KW_DO,
    KW_ELIF,
    KW_ELSE,
    KW_FINAL,
    KW_FINALLY,
    KW_FOR,
    KW_IF,
    KW_IMPORT,
    KW_RETURN,
    KW_STATIC,
    KW_SWITCH,
    KW_TRY,
    KW_WHILE,

    TYPE_ANY,
    TYPE_BOOL,
    TYPE_BYTE,
    TYPE_CHAR,
    TYPE_CLASS,
    TYPE_DOUBLE,
    TYPE_FLOAT,
    TYPE_FUNC,
    TYPE_INT,
    TYPE_LONG,
    TYPE_SHORT,
    TYPE_STR,
    TYPE_VOID,
  };

  std::optional<ReservedToken> getKeyword(std::string_view word);
  std::optional<ReservedToken> getOperator(PushBackStream& stream);

  struct Identifier {
    std::string name;
  };

  bool operator==(const Identifier& id1, const Identifier& id2);
  bool operator!=(const Identifier& id1, const Identifier& id2);

  struct Eof {
  };

  bool operator==(const Eof&, const Eof&);
  bool operator!=(const Eof&, const Eof&);

  struct VoidValue {
  };

  bool operator==(const VoidValue&, const VoidValue&);
  bool operator!=(const VoidValue&, const VoidValue&);

  using TokenValue = std::variant<
    Eof,
    ReservedToken,
    Identifier,
    VoidValue, // void (null)
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

  class Token {
  private:
    TokenValue _value;
    size_t _line_number;
    size_t _char_index;
  
  public:
    Token(TokenValue value, size_t lineNumber, size_t charIndex);

    bool isEof() const;
    bool isReservedToken() const;
    bool isIdentifier() const;
    bool isNull() const;
    bool isByte() const;
    bool isShort() const;
    bool isInt() const;
    bool isLong() const;
    bool isFloat() const;
    bool isDouble() const;
    bool isBool() const;
    bool isChar() const;
    bool isStr() const;

    bool isLiteral() const;
    bool isIntegral() const;
    bool isNumeric() const;

    const TokenValue& getValue() const;

    ReservedToken getReservedToken() const;
    const Identifier& getIdentifier() const;
    int8_t getByte() const;
    int16_t getShort() const;
    int32_t getInt() const;
    int64_t getLong() const;
    float getFloat() const;
    double getDouble() const;
    bool getBool() const;
    char getChar() const;
    const std::string& getStr() const;

    size_t lineNumber() const;
    size_t charIndex() const;

    bool hasValue(TokenValue value) const;

    std::string toString() const;
  };

  std::string reservedTokenRepr(ReservedToken t);

}