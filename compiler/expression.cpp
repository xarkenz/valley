#include "expression.hpp"

#include "errors.hpp"
#include "util.hpp"


namespace valley {

  namespace {

    bool isConvertible(TypeHandle typeFrom, bool lvalueFrom, TypeHandle typeTo, bool lvalueTo) {
      if ((typeFrom == typeTo && lvalueFrom == lvalueTo) || typeTo == TypeRegistry::voidHandle())
        return true;
      if (lvalueTo)
        return lvalueFrom && isConvertible(typeFrom, false, typeTo, false);
      if (std::holds_alternative<AnyType>(*typeTo) || typeTo == TypeRegistry::boolHandle())
        return true;
      if (std::holds_alternative<ArrayType>(*typeFrom) && std::holds_alternative<ArrayType>(*typeTo))
        return isConvertible(std::get<ArrayType>(*typeFrom).inner, lvalueFrom, std::get<ArrayType>(*typeTo).inner, lvalueTo);
      if (typeFrom == TypeRegistry::boolHandle())
        return typeTo == TypeRegistry::byteHandle()
          || typeTo == TypeRegistry::shortHandle()
          || typeTo == TypeRegistry::intHandle()
          || typeTo == TypeRegistry::longHandle()
          || typeTo == TypeRegistry::floatHandle()
          || typeTo == TypeRegistry::doubleHandle()
          || typeTo == TypeRegistry::charHandle();
      if (typeFrom == TypeRegistry::byteHandle() || typeFrom == TypeRegistry::charHandle())
        return typeTo == TypeRegistry::byteHandle()
          || typeTo == TypeRegistry::shortHandle()
          || typeTo == TypeRegistry::intHandle()
          || typeTo == TypeRegistry::longHandle()
          || typeTo == TypeRegistry::floatHandle()
          || typeTo == TypeRegistry::doubleHandle()
          || typeTo == TypeRegistry::charHandle();
      if (typeFrom == TypeRegistry::shortHandle())
        return typeTo == TypeRegistry::shortHandle()
          || typeTo == TypeRegistry::intHandle()
          || typeTo == TypeRegistry::longHandle()
          || typeTo == TypeRegistry::floatHandle()
          || typeTo == TypeRegistry::doubleHandle();
      if (typeFrom == TypeRegistry::intHandle())
        return typeTo == TypeRegistry::intHandle()
          || typeTo == TypeRegistry::longHandle()
          || typeTo == TypeRegistry::floatHandle()
          || typeTo == TypeRegistry::doubleHandle();
      if (typeFrom == TypeRegistry::longHandle())
        return typeTo == TypeRegistry::longHandle()
          || typeTo == TypeRegistry::floatHandle()
          || typeTo == TypeRegistry::doubleHandle();
      if (typeFrom == TypeRegistry::floatHandle())
        return typeTo == TypeRegistry::floatHandle()
          || typeTo == TypeRegistry::doubleHandle();
      if (typeFrom == TypeRegistry::doubleHandle())
        return typeTo == TypeRegistry::doubleHandle();
      return typeTo == TypeRegistry::strHandle();
    }

    bool isNumeric(TypeHandle t) {
      return t == TypeRegistry::doubleHandle()
        || t == TypeRegistry::floatHandle()
        || t == TypeRegistry::longHandle()
        || t == TypeRegistry::intHandle()
        || t == TypeRegistry::shortHandle()
        || t == TypeRegistry::byteHandle()
        || t == TypeRegistry::charHandle()
        || t == TypeRegistry::boolHandle();
    }

    TypeHandle maxNumericPrecision(TypeHandle t1, TypeHandle t2) {
      if (!isNumeric(t1) || !isNumeric(t2))
        return TypeRegistry::voidHandle();
      if (t1 == TypeRegistry::doubleHandle() || t2 == TypeRegistry::doubleHandle())
        return TypeRegistry::doubleHandle();
      if (t1 == TypeRegistry::floatHandle() || t2 == TypeRegistry::floatHandle())
        return TypeRegistry::floatHandle();
      if (t1 == TypeRegistry::longHandle() || t2 == TypeRegistry::longHandle())
        return TypeRegistry::longHandle();
      if (t1 == TypeRegistry::intHandle() || t2 == TypeRegistry::intHandle())
        return TypeRegistry::intHandle();
      if (t1 == TypeRegistry::shortHandle() || t2 == TypeRegistry::shortHandle())
        return TypeRegistry::shortHandle();
      if (t1 == TypeRegistry::byteHandle() || t2 == TypeRegistry::byteHandle())
        return TypeRegistry::byteHandle();
      if (t1 == TypeRegistry::charHandle() || t2 == TypeRegistry::charHandle())
        return TypeRegistry::charHandle();
      if (t1 == TypeRegistry::boolHandle() || t2 == TypeRegistry::boolHandle())
        return TypeRegistry::boolHandle();
      return TypeRegistry::voidHandle();
    }
  }

  Expression::Expression(CompilerContext& context, ExpressionValue value, std::vector<Expression::Ptr> children, size_t lineNumber, size_t charIndex):
    _value(std::move(value)),
    _children(std::move(children)),
    _line_number(lineNumber),
    _char_index(charIndex)
  {
    const TypeHandle any_handle = TypeRegistry::anyHandle();
    const TypeHandle voidHandle = TypeRegistry::voidHandle();
    const TypeHandle byteHandle = TypeRegistry::byteHandle();
    const TypeHandle shortHandle = TypeRegistry::shortHandle();
    const TypeHandle intHandle = TypeRegistry::intHandle();
    const TypeHandle longHandle = TypeRegistry::longHandle();
    const TypeHandle floatHandle = TypeRegistry::floatHandle();
    const TypeHandle doubleHandle = TypeRegistry::doubleHandle();
    const TypeHandle boolHandle = TypeRegistry::boolHandle();
    const TypeHandle charHandle = TypeRegistry::charHandle();
    const TypeHandle strHandle = TypeRegistry::strHandle();

    std::visit(Overloaded{
      [&](VoidValue) {
        _type = voidHandle;
        _lvalue = false;
      },
      [&](int8_t value) {
        _type = byteHandle;
        _lvalue = false;
      },
      [&](int16_t value) {
        _type = shortHandle;
        _lvalue = false;
      },
      [&](int32_t value) {
        _type = intHandle;
        _lvalue = false;
      },
      [&](int64_t value) {
        _type = longHandle;
        _lvalue = false;
      },
      [&](float value) {
        _type = floatHandle;
        _lvalue = false;
      },
      [&](double value) {
        _type = doubleHandle;
        _lvalue = false;
      },
      [&](bool value) {
        _type = boolHandle;
        _lvalue = false;
      },
      [&](char value) {
        _type = charHandle;
        _lvalue = false;
      },
      [&](const std::string& value) {
        _type = strHandle;
        _lvalue = false;
      },
      [&](const Identifier& value) {
        if (const IdentifierInfo* info = context.find(value.name)) {
          _type = info->type();
          _lvalue = !info->isFinal();
        } else
          throw SemanticError("Identifier '" + value.name + "' may not have been declared in this scope.", _line_number, _char_index, 0);
      },
      [&](const Declaration& value) {
        if (const IdentifierInfo* info = context.find(value.name))
          throw SemanticError("Identifier '" + value.name + "' may already be declared in this scope.", _line_number, _char_index, 0);
        else {
          _type = value.type;
          _lvalue = !value.isFinal;
        }
      },
      [&](Operation value) {
        switch (value) {
          case Operation::INC_BEFORE:
          case Operation::INC_AFTER:
          case Operation::DEC_BEFORE:
          case Operation::DEC_AFTER:
            _children[0]->checkConversion(doubleHandle, true);
            _type = _children[0]->_type;
            _lvalue = true;
            break;
          case Operation::POS:
          case Operation::NEG:
            _children[0]->checkConversion(doubleHandle, false);
            _type = _children[0]->_type;
            _lvalue = false;
            break;
          case Operation::NOT:
            _children[0]->checkConversion(longHandle, false);
            _type = _children[0]->_type;
            _lvalue = false;
            break;
          case Operation::LNOT:
            _children[0]->checkConversion(boolHandle, false);
            _type = boolHandle;
            _lvalue = false;
            break;
          case Operation::ADD:
          case Operation::SUB:
          case Operation::MUL:
          case Operation::DIV:
          case Operation::MOD:
          case Operation::POW:
            _children[0]->checkConversion(doubleHandle, false);
            _children[1]->checkConversion(doubleHandle, false);
            _type = maxNumericPrecision(_children[0]->type(), _children[1]->type());
            _lvalue = false;
            break;
          case Operation::AND:
          case Operation::OR:
          case Operation::XOR:
          case Operation::LSHIFT:
          case Operation::RSHIFT:
            _children[0]->checkConversion(longHandle, false);
            _children[1]->checkConversion(longHandle, false);
            _type = maxNumericPrecision(_children[0]->type(), _children[1]->type());
            _lvalue = false;
            break;
          case Operation::LAND:
          case Operation::LOR:
          case Operation::LXOR:
            _children[0]->checkConversion(boolHandle, false);
            _children[1]->checkConversion(boolHandle, false);
            _type = boolHandle;
            _lvalue = false;
            break;
          case Operation::LT:
          case Operation::GT:
          case Operation::LTEQ:
          case Operation::GTEQ:
            _children[0]->checkConversion(doubleHandle, false);
            _children[1]->checkConversion(doubleHandle, false);
          case Operation::EQ:
          case Operation::NEQ:
            _type = boolHandle;
            _lvalue = false;
            break;
          case Operation::SET:
            _type = _children[0]->type();
            _lvalue = true;
            _children[0]->checkConversion(_type, true);
            _children[1]->checkConversion(_type, false);
            break;
          case Operation::SET_ADD:
          case Operation::SET_SUB:
          case Operation::SET_MUL:
          case Operation::SET_DIV:
          case Operation::SET_MOD:
          case Operation::SET_POW:
            _children[0]->checkConversion(doubleHandle, true);
            _children[1]->checkConversion(doubleHandle, false);
            _type = _children[0]->type();
            _lvalue = true;
            break;
          case Operation::SET_AND:
          case Operation::SET_OR:
          case Operation::SET_XOR:
          case Operation::SET_LSHIFT:
          case Operation::SET_RSHIFT:
            _children[0]->checkConversion(longHandle, true);
            _children[1]->checkConversion(longHandle, false);
            _type = _children[0]->type();
            _lvalue = true;
            break;
          case Operation::COMMA:
            _type = _children.back()->type();
            _lvalue = _children.back()->lvalue();
            break;
          case Operation::SUBSCRIPT:
            if (const ArrayType* at = std::get_if<ArrayType>(_children[0]->type())) {
              _type = at->inner;
              _lvalue = _children[0]->lvalue();
            } else if (_children[0]->type() == strHandle) {
              _type = charHandle;
              _lvalue = false;
            } else {
              throw SemanticError(typeHandleRepr(_children[0]->type()) + " is not subscriptable.", _line_number, _char_index, 0);
            }
            break;
          case Operation::TERNARY:
            _children[0]->checkConversion(boolHandle, false);
            if (isConvertible(
              _children[2]->type(), _children[2]->lvalue(),
              _children[1]->type(), _children[1]->lvalue()
            )) {
              _children[2]->checkConversion(_children[1]->type(), _children[1]->lvalue());
              _type = _children[1]->type();
              _lvalue = _children[1]->lvalue();
            } else {
              _children[1]->checkConversion(_children[2]->type(), _children[2]->lvalue());
              _type = _children[2]->type();
              _lvalue = _children[2]->lvalue();
            }
            break;
          case Operation::CALL:
            if (const FuncType* ft = std::get_if<FuncType>(_children[0]->type())) {
              _type = ft->returnType;
              _lvalue = false;
              if (ft->hasArgCatch) {
                if (const ArrayType* varargs = std::get_if<ArrayType>(ft->paramTypes.back()))
                for (size_t i = 0; i < _children.size() - 1; ++i) {
                  if (i >= ft->paramTypes.size() - 1)
                    _children[i + 1]->checkConversion(varargs->inner, false);
                  else
                    _children[i + 1]->checkConversion(ft->paramTypes[i], false);
                } else
                  throw CompileError("varargs not working properly in function signature.", _line_number, _char_index, 1);
              } else {
                if (ft->paramTypes.size() != _children.size() - 1)
                  throw SemanticError("expected " + std::to_string(ft->paramTypes.size()) + " arguments, got " + std::to_string(_children.size() - 1) + " instead.", _line_number, _char_index, 0);
                for (size_t i = 0; i < ft->paramTypes.size(); ++i) {
                  _children[i + 1]->checkConversion(ft->paramTypes[i], false);
                }
              }
            } else
              throw SemanticError("'" + typeHandleRepr(_children[0]->_type) + "' object is not callable.", _line_number, _char_index, 0);
            break;
          case Operation::ARRAY:
            _type = _children.back()->type(); // Type to check against (temporary)
            for (int i = 0; i < int(_children.size()) - 1; ++i) {
              _children[i]->checkConversion(_type, false);
            }
            _type = context.getHandle(ArrayType{_type});
            _lvalue = false;
        }
      }
    }, _value);
  }

  bool Expression::isOperation() const {
    return std::holds_alternative<Operation>(_value);
  }

  bool Expression::isIdentifier() const {
    return std::holds_alternative<Identifier>(_value);
  }

  bool Expression::isDeclaration() const {
    return std::holds_alternative<Declaration>(_value);
  }

  bool Expression::isNull() const {
    return std::holds_alternative<VoidValue>(_value);
  }

  bool Expression::isByte() const {
    return std::holds_alternative<int8_t>(_value);
  }

  bool Expression::isShort() const {
    return std::holds_alternative<int16_t>(_value);
  }

  bool Expression::isInt() const {
    return std::holds_alternative<int32_t>(_value);
  }

  bool Expression::isLong() const {
    return std::holds_alternative<int64_t>(_value);
  }

  bool Expression::isFloat() const {
    return std::holds_alternative<float>(_value);
  }

  bool Expression::isDouble() const {
    return std::holds_alternative<double>(_value);
  }

  bool Expression::isBool() const {
    return std::holds_alternative<bool>(_value);
  }

  bool Expression::isChar() const {
    return std::holds_alternative<char>(_value);
  }

  bool Expression::isStr() const {
    return std::holds_alternative<std::string>(_value);
  }

  const ExpressionValue& Expression::getValue() const {
    return _value;
  }

  Operation Expression::getOperation() const {
    return std::get<Operation>(_value);
  }

  Identifier Expression::getIdentifier() const {
    return std::get<Identifier>(_value);
  }

  Declaration Expression::getDeclaration() const {
    return std::get<Declaration>(_value);
  }

  int8_t Expression::getByte() const {
    return std::get<int8_t>(_value);
  }

  int16_t Expression::getShort() const {
    return std::get<int16_t>(_value);
  }

  int32_t Expression::getInt() const {
    return std::get<int32_t>(_value);
  }

  int64_t Expression::getLong() const {
    return std::get<int64_t>(_value);
  }

  float Expression::getFloat() const {
    return std::get<float>(_value);
  }

  double Expression::getDouble() const {
    return std::get<double>(_value);
  }

  bool Expression::getBool() const {
    return std::get<bool>(_value);
  }

  char Expression::getChar() const {
    return std::get<char>(_value);
  }

  std::string_view Expression::getStr() const {
    return std::get<std::string>(_value);
  }

  const std::vector<Expression::Ptr>& Expression::children() const {
    return _children;
  }

  TypeHandle Expression::type() const {
    return _type;
  }

  bool Expression::lvalue() const {
    return _lvalue;
  }

  size_t Expression::lineNumber() const {
    return _line_number;
  }

  size_t Expression::charIndex() const {
    return _char_index;
  }

  void Expression::checkConversion(TypeHandle type, bool lvalue) const {
    if (!isConvertible(_type, _lvalue, type, lvalue)) {
      if (!isConvertible(_type, _lvalue, type, false))
        throw TypeError(typeHandleRepr(_type), typeHandleRepr(type), _line_number, _char_index, 0);
      throw SemanticError("cannot be assigned to.", _line_number, _char_index, 0);
    }
  }

  std::string expressionRepr(const Expression::Ptr& expr) {
    return visit(Overloaded{
      [&](int8_t num) -> std::string {
        return std::to_string(num);
      },
      [&](int16_t num) -> std::string {
        return std::to_string(num);
      },
      [&](int32_t num) -> std::string {
        return std::to_string(num);
      },
      [&](int64_t num) -> std::string {
        return std::to_string(num);
      },
      [&](float num) -> std::string {
        return std::to_string(num);
      },
      [&](double num) -> std::string {
        return std::to_string(num);
      },
      [&](bool b) -> std::string {
        return b ? "true" : "false";
      },
      [&](char c) -> std::string {
        return "'" + std::to_string(c) + "'";
      },
      [&](const std::string& str) -> std::string {
        return '"' + str + '"';
      },
      [&](const Identifier& id) -> std::string {
        return "$" + id.name;
      },
      [&](const Declaration& dec) -> std::string {
        return "(" + std::string(dec.isFinal ? "final " : "") + std::string(dec.isStatic ? "static " : "") + typeHandleRepr(dec.type) + " $" + dec.name + ")";
      },
      [&](Operation op) -> std::string {
        switch (op) {
          case Operation::INC_BEFORE:
            return "(++" + expressionRepr(expr->children()[0]) + ")";
          case Operation::INC_AFTER:
            return "(" + expressionRepr(expr->children()[0]) + ")++";
          case Operation::DEC_BEFORE:
            return "(--" + expressionRepr(expr->children()[0]) + ")";
          case Operation::DEC_AFTER:
            return "(" + expressionRepr(expr->children()[0]) + "--)";
          case Operation::POS:
            return "(+" + expressionRepr(expr->children()[0]) + ")";
          case Operation::NEG:
            return "(-" + expressionRepr(expr->children()[0]) + ")";
          case Operation::NOT:
            return "(~" + expressionRepr(expr->children()[0]) + ")";
          case Operation::LNOT:
            return "(!" + expressionRepr(expr->children()[0]) + ")";
          case Operation::ADD:
            return "(" + expressionRepr(expr->children()[0]) + " + " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SUB:
            return "(" + expressionRepr(expr->children()[0]) + " - " + expressionRepr(expr->children()[1]) + ")";
          case Operation::MUL:
            return "(" + expressionRepr(expr->children()[0]) + " * " + expressionRepr(expr->children()[1]) + ")";
          case Operation::DIV:
            return "(" + expressionRepr(expr->children()[0]) + " / " + expressionRepr(expr->children()[1]) + ")";
          case Operation::MOD:
            return "(" + expressionRepr(expr->children()[0]) + " % " + expressionRepr(expr->children()[1]) + ")";
          case Operation::POW:
            return "(" + expressionRepr(expr->children()[0]) + " ** " + expressionRepr(expr->children()[1]) + ")";
          case Operation::AND:
            return "(" + expressionRepr(expr->children()[0]) + " & " + expressionRepr(expr->children()[1]) + ")";
          case Operation::OR:
            return "(" + expressionRepr(expr->children()[0]) + " | " + expressionRepr(expr->children()[1]) + ")";
          case Operation::XOR:
            return "(" + expressionRepr(expr->children()[0]) + " ^ " + expressionRepr(expr->children()[1]) + ")";
          case Operation::LSHIFT:
            return "(" + expressionRepr(expr->children()[0]) + ") + " + expressionRepr(expr->children()[1]) + ")";
          case Operation::RSHIFT:
            return "(" + expressionRepr(expr->children()[0]) + " >> " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET:
            return "(" + expressionRepr(expr->children()[0]) + " = " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_ADD:
            return "(" + expressionRepr(expr->children()[0]) + " += " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_SUB:
            return "(" + expressionRepr(expr->children()[0]) + " -= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_MUL:
            return "(" + expressionRepr(expr->children()[0]) + " *= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_DIV:
            return "(" + expressionRepr(expr->children()[0]) + " /= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_MOD:
            return "(" + expressionRepr(expr->children()[0]) + " %= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_POW:
            return "(" + expressionRepr(expr->children()[0]) + " **= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_AND:
            return "(" + expressionRepr(expr->children()[0]) + " &= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_OR:
            return "(" + expressionRepr(expr->children()[0]) + " |= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_XOR:
            return "(" + expressionRepr(expr->children()[0]) + " ^= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_LSHIFT:
            return "(" + expressionRepr(expr->children()[0]) + ") <<= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SET_RSHIFT:
            return "(" + expressionRepr(expr->children()[0]) + " >>= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::EQ:
            return "(" + expressionRepr(expr->children()[0]) + " == " + expressionRepr(expr->children()[1]) + ")";
          case Operation::NEQ:
            return "(" + expressionRepr(expr->children()[0]) + " != " + expressionRepr(expr->children()[1]) + ")";
          case Operation::LT:
            return "(" + expressionRepr(expr->children()[0]) + " < " + expressionRepr(expr->children()[1]) + ")";
          case Operation::GT:
            return "(" + expressionRepr(expr->children()[0]) + " > " + expressionRepr(expr->children()[1]) + ")";
          case Operation::LTEQ:
            return "(" + expressionRepr(expr->children()[0]) + " <= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::GTEQ:
            return "(" + expressionRepr(expr->children()[0]) + " >= " + expressionRepr(expr->children()[1]) + ")";
          case Operation::COMMA:
            return "(" + expressionRepr(expr->children()[0]) + ", " + expressionRepr(expr->children()[1]) + ")";
          case Operation::LAND:
            return "(" + expressionRepr(expr->children()[0]) + " && " + expressionRepr(expr->children()[1]) + ")";
          case Operation::LOR:
            return "(" + expressionRepr(expr->children()[0]) + " || " + expressionRepr(expr->children()[1]) + ")";
          case Operation::LXOR:
            return "(" + expressionRepr(expr->children()[0]) + " ^^ " + expressionRepr(expr->children()[1]) + ")";
          case Operation::SUBSCRIPT:
            return "(" + expressionRepr(expr->children()[0]) + "[" + expressionRepr(expr->children()[1]) + "])";
          case Operation::TERNARY:
            return "(" + expressionRepr(expr->children()[0]) + " ? " + expressionRepr(expr->children()[1]) + " : " + expressionRepr(expr->children()[2]) + ")";
          case Operation::CALL: {
            std::string str = expressionRepr(expr->children()[0]) + "(";
            const char* separator = "";
            for (size_t i = 1; i < expr->children().size(); ++i) {
              str += separator + expressionRepr(expr->children()[i]);
              separator = ", ";
            }
            return str + ")";
          }
          case Operation::ARRAY: {
            std::string str = "[";
            const char* separator = "";
            for (size_t i = 0; i < expr->children().size(); ++i) {
              str += separator + expressionRepr(expr->children()[i]);
              separator = ", ";
            }
            return str + "]";
          }
        }
      },
      [&](const auto&) -> std::string {
        return "";
      }
    }, expr->getValue());
  }

}