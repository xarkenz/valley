#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "compiler_context.hpp"
#include "tokens.hpp"
#include "types.hpp"


namespace valley {

  enum struct Operation {
    // Unary
    INC_BEFORE,
    INC_AFTER,
    DEC_BEFORE,
    DEC_AFTER,
    POS,
    NEG,
    NOT,
    LNOT,

    // Binary
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,
    AND,
    OR,
    XOR,
    LSHIFT,
    RSHIFT,
    SET,
    SET_ADD,
    SET_SUB,
    SET_MUL,
    SET_DIV,
    SET_MOD,
    SET_POW,
    SET_AND,
    SET_OR,
    SET_XOR,
    SET_LSHIFT,
    SET_RSHIFT,
    EQ,
    NEQ,
    LT,
    GT,
    LTEQ,
    GTEQ,
    LAND,
    LOR,
    LXOR,

    // Other
    TERNARY,
    COMMA,
    SUBSCRIPT,
    CALL,
    ARRAY,
  };

  struct Declaration {
    std::string name;
    TypeHandle type;
    bool isFinal;
    bool isStatic;
  };

  using ExpressionValue = std::variant<Operation, VoidValue, int8_t, int16_t, int32_t, int64_t, float, double, bool, char, std::string, Identifier, Declaration>;
  
  struct Expression {
  public:
    using Ptr = std::unique_ptr<Expression>;

  private:
    ExpressionValue _value;
    std::vector<Expression::Ptr> _children;
    TypeHandle _type;
    bool _lvalue;
    size_t _line_number;
    size_t _char_index;

  public:
    Expression(CompilerContext& context, ExpressionValue value, std::vector<Expression::Ptr> children, size_t lineNumber, size_t charIndex);

    bool isOperation() const;
    bool isIdentifier() const;
    bool isDeclaration() const;
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

    const ExpressionValue& getValue() const;

    Operation getOperation() const;
    Identifier getIdentifier() const;
    Declaration getDeclaration() const;
    int8_t getByte() const;
    int16_t getShort() const;
    int32_t getInt() const;
    int64_t getLong() const;
    float getFloat() const;
    double getDouble() const;
    bool getBool() const;
    char getChar() const;
    std::string_view getStr() const;
    
    const std::vector<Expression::Ptr>& children() const;
    TypeHandle type() const;
    bool lvalue() const;
    size_t lineNumber() const;
    size_t charIndex() const;

    void checkConversion(TypeHandle type, bool lvalue) const;

    //const VariableValue& evaluate(RuntimeContext& context) const;
  };

  std::string expressionRepr(const Expression::Ptr& expr);

}