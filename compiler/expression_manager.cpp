#include "expression_manager.hpp"

#include "errors.hpp"
#include "tokenizer.hpp"

#include <iostream> // buh


namespace valley {

  namespace {

    enum struct OperatorPrecedence {
      POSTFIX,        // x++  x--  x[...]  x(...)
      PREFIX,         // ++x  --x  +x  -x  ~x  !x
      EXPONENTATIVE,  // x**y
      MULTIPLICATIVE, // x*y  x/y  x%y
      ADDITIVE,       // x+y  x-y
      SHIFT,          // x<<y  x>>y
      INEQUALITY,     // x<y  x>y  x<=y  x>=y
      EQUALITY,       // x==y  x!=y
      BITWISE_AND,    // x&y
      BITWISE_XOR,    // x^y
      BITWISE_OR,     // x|y
      LOGICAL_AND,    // x&&y
      LOGICAL_XOR,    // x^^y
      LOGICAL_OR,     // x||y
      ASSIGNMENT,     // x=y  x+=y  x-=y  x*=y  x/=y  x%=y  x**=y  x&=y  x|=y  x^=y  x<<=y  x>>=y  x?y:z
      COMMA,          // x,y
      CONTAINER,      // [...]
    };

    enum struct OperatorAssociativity {
      LeftToRight,
      RightToLeft,
    };

    struct OperatorInfo {
      Operation operation;
      OperatorPrecedence precedence;
      OperatorAssociativity associativity;
      int numOperands;
      size_t lineNumber;
      size_t charIndex;

      OperatorInfo(Operation operation, size_t lineNumber, size_t charIndex):
      operation(operation), lineNumber(lineNumber), charIndex(charIndex) {
        switch (operation) {
          case Operation::INC_AFTER:
          case Operation::DEC_AFTER:
          case Operation::SUBSCRIPT:
          case Operation::CALL:
            precedence = OperatorPrecedence::POSTFIX;
            break;

          case Operation::INC_BEFORE:
          case Operation::DEC_BEFORE:
          case Operation::POS:
          case Operation::NEG:
          case Operation::NOT:
          case Operation::LNOT:
            precedence = OperatorPrecedence::PREFIX;
            break;

          case Operation::POW:
            precedence = OperatorPrecedence::EXPONENTATIVE;
            break;

          case Operation::MUL:
          case Operation::DIV:
          case Operation::MOD:
            precedence = OperatorPrecedence::MULTIPLICATIVE;
            break;

          case Operation::ADD:
          case Operation::SUB:
            precedence = OperatorPrecedence::ADDITIVE;
            break;

          case Operation::LSHIFT:
          case Operation::RSHIFT:
            precedence = OperatorPrecedence::SHIFT;
            break;

          case Operation::LT:
          case Operation::GT:
          case Operation::LTEQ:
          case Operation::GTEQ:
            precedence = OperatorPrecedence::INEQUALITY;
            break;

          case Operation::EQ:
          case Operation::NEQ:
            precedence = OperatorPrecedence::EQUALITY;
            break;

          case Operation::AND:
            precedence = OperatorPrecedence::BITWISE_AND;
            break;

          case Operation::XOR:
            precedence = OperatorPrecedence::BITWISE_XOR;
            break;

          case Operation::OR:
            precedence = OperatorPrecedence::BITWISE_OR;
            break;

          case Operation::LAND:
            precedence = OperatorPrecedence::LOGICAL_AND;
            break;

          case Operation::LXOR:
            precedence = OperatorPrecedence::LOGICAL_XOR;
            break;

          case Operation::LOR:
            precedence = OperatorPrecedence::LOGICAL_OR;
            break;

          case Operation::SET:
          case Operation::SET_ADD:
          case Operation::SET_SUB:
          case Operation::SET_MUL:
          case Operation::SET_DIV:
          case Operation::SET_MOD:
          case Operation::SET_POW:
          case Operation::SET_AND:
          case Operation::SET_OR:
          case Operation::SET_XOR:
          case Operation::SET_LSHIFT:
          case Operation::SET_RSHIFT:
          case Operation::TERNARY:
            precedence = OperatorPrecedence::ASSIGNMENT;
            break;

          case Operation::COMMA:
            precedence = OperatorPrecedence::COMMA;
            break;

          case Operation::ARRAY:
            precedence = OperatorPrecedence::CONTAINER;
            break;
        }

        switch (precedence) {
          case OperatorPrecedence::ASSIGNMENT:
          case OperatorPrecedence::PREFIX:
            associativity = OperatorAssociativity::RightToLeft;
            break;

          default:
            associativity = OperatorAssociativity::LeftToRight;
            break;
        }

        switch (operation) {
          case Operation::INC_BEFORE:
          case Operation::INC_AFTER:
          case Operation::DEC_BEFORE:
          case Operation::DEC_AFTER:
          case Operation::POS:
          case Operation::NEG:
          case Operation::NOT:
          case Operation::LNOT:
          case Operation::COMMA:
          case Operation::CALL:
          case Operation::ARRAY:
            numOperands = 1; // Unary
            break;

          case Operation::TERNARY:
            numOperands = 3; // Ternary
            break;

          default:
            numOperands = 2; // Binary
            break;
        }
      }
    };

    OperatorInfo getOperatorInfo(ReservedToken rtoken, bool prefix, size_t lineNumber, size_t charIndex) {
      switch (rtoken) {
        case ReservedToken::D_PLUS:
          return prefix
            ? OperatorInfo(Operation::INC_BEFORE, lineNumber, charIndex)
            : OperatorInfo(Operation::INC_AFTER, lineNumber, charIndex);

        case ReservedToken::D_HYPHEN:
          return prefix
            ? OperatorInfo(Operation::DEC_BEFORE, lineNumber, charIndex)
            : OperatorInfo(Operation::DEC_AFTER, lineNumber, charIndex);

        case ReservedToken::PLUS:
          return prefix
            ? OperatorInfo(Operation::POS, lineNumber, charIndex)
            : OperatorInfo(Operation::ADD, lineNumber, charIndex);

        case ReservedToken::HYPHEN:
          return prefix
            ? OperatorInfo(Operation::NEG, lineNumber, charIndex)
            : OperatorInfo(Operation::SUB, lineNumber, charIndex);

        case ReservedToken::ASTERISK:
          return OperatorInfo(Operation::MUL, lineNumber, charIndex);

        case ReservedToken::SLASH:
          return OperatorInfo(Operation::DIV, lineNumber, charIndex);

        case ReservedToken::PERCENT:
          return OperatorInfo(Operation::MOD, lineNumber, charIndex);

        case ReservedToken::D_ASTERISK:
          return OperatorInfo(Operation::POW, lineNumber, charIndex);

        case ReservedToken::TILDE:
          return OperatorInfo(Operation::NOT, lineNumber, charIndex);

        case ReservedToken::AMPERSAND:
          return OperatorInfo(Operation::AND, lineNumber, charIndex);

        case ReservedToken::BAR:
          return OperatorInfo(Operation::OR, lineNumber, charIndex);

        case ReservedToken::CARET:
          return OperatorInfo(Operation::XOR, lineNumber, charIndex);

        case ReservedToken::D_ANGLE_L:
          return OperatorInfo(Operation::LSHIFT, lineNumber, charIndex);

        case ReservedToken::D_ANGLE_R:
          return OperatorInfo(Operation::RSHIFT, lineNumber, charIndex);

        case ReservedToken::EQUAL:
          return OperatorInfo(Operation::SET, lineNumber, charIndex);

        case ReservedToken::PLUS_EQUAL:
          return OperatorInfo(Operation::SET_ADD, lineNumber, charIndex);

        case ReservedToken::HYPHEN_EQUAL:
          return OperatorInfo(Operation::SET_SUB, lineNumber, charIndex);

        case ReservedToken::ASTERISK_EQUAL:
          return OperatorInfo(Operation::SET_MUL, lineNumber, charIndex);

        case ReservedToken::SLASH_EQUAL:
          return OperatorInfo(Operation::SET_DIV, lineNumber, charIndex);

        case ReservedToken::PERCENT_EQUAL:
          return OperatorInfo(Operation::SET_MOD, lineNumber, charIndex);

        case ReservedToken::D_ASTERISK_EQUAL:
          return OperatorInfo(Operation::SET_POW, lineNumber, charIndex);

        case ReservedToken::AMPERSAND_EQUAL:
          return OperatorInfo(Operation::SET_AND, lineNumber, charIndex);

        case ReservedToken::BAR_EQUAL:
          return OperatorInfo(Operation::SET_OR, lineNumber, charIndex);

        case ReservedToken::CARET_EQUAL:
          return OperatorInfo(Operation::SET_XOR, lineNumber, charIndex);

        case ReservedToken::D_ANGLE_L_EQUAL:
          return OperatorInfo(Operation::SET_LSHIFT, lineNumber, charIndex);

        case ReservedToken::D_ANGLE_R_EQUAL:
          return OperatorInfo(Operation::SET_RSHIFT, lineNumber, charIndex);

        case ReservedToken::EXCLAMATION:
          return OperatorInfo(Operation::LNOT, lineNumber, charIndex);

        case ReservedToken::D_AMPERSAND:
          return OperatorInfo(Operation::LAND, lineNumber, charIndex);

        case ReservedToken::D_BAR:
          return OperatorInfo(Operation::LOR, lineNumber, charIndex);

        case ReservedToken::D_CARET:
          return OperatorInfo(Operation::LXOR, lineNumber, charIndex);

        case ReservedToken::D_EQUAL:
          return OperatorInfo(Operation::EQ, lineNumber, charIndex);

        case ReservedToken::EXCLAMATION_EQUAL:
          return OperatorInfo(Operation::NEQ, lineNumber, charIndex);

        case ReservedToken::ANGLE_L:
          return OperatorInfo(Operation::LT, lineNumber, charIndex);

        case ReservedToken::ANGLE_R:
          return OperatorInfo(Operation::GT, lineNumber, charIndex);

        case ReservedToken::ANGLE_L_EQUAL:
          return OperatorInfo(Operation::LTEQ, lineNumber, charIndex);

        case ReservedToken::ANGLE_R_EQUAL:
          return OperatorInfo(Operation::GTEQ, lineNumber, charIndex);

        case ReservedToken::QUESTION:
          return OperatorInfo(Operation::TERNARY, lineNumber, charIndex);

        case ReservedToken::COMMA:
          return OperatorInfo(Operation::COMMA, lineNumber, charIndex);

        case ReservedToken::ROUND_L:
          return OperatorInfo(Operation::CALL, lineNumber, charIndex);

        case ReservedToken::SQUARE_L:
          return OperatorInfo(Operation::SUBSCRIPT, lineNumber, charIndex);

        default:
          throw SyntaxError_unexpected(reservedTokenRepr(rtoken), lineNumber, charIndex, false);
      }
    }

    bool isExpressionEnd(const Token& t, bool allowComma) {
      if (t.isReservedToken()) {
        switch (t.getReservedToken()) {
          case ReservedToken::SEMICOLON:
          case ReservedToken::COLON:
          case ReservedToken::ROUND_R:
          case ReservedToken::SQUARE_R:
          case ReservedToken::CURLY_R:
            return true;

          case ReservedToken::COMMA:
            return !allowComma;

          default:
            return false;
        }
      }
      return t.isEof();
    }

    bool isEvaluatedBefore(const OperatorInfo& l, const OperatorInfo& r) {
      return l.associativity == OperatorAssociativity::LeftToRight ? l.precedence <= r.precedence : l.precedence < r.precedence;
    }

    void popOperator(std::stack<OperatorInfo>& operatorStack, std::stack<Expression::Ptr>& operandStack, CompilerContext& context, size_t lineNumber, size_t charIndex) {
      if (operandStack.size() < operatorStack.top().numOperands) {
        std::string str = "failed to parse expression. (expected ";
        str += std::to_string(operatorStack.top().numOperands);
        str += " operands, got ";
        str += std::to_string(operandStack.size());
        throw CompileError(str + ".)", lineNumber, charIndex, 0);
      }

      std::vector<Expression::Ptr> operands;
      operands.resize(operatorStack.top().numOperands);

      if (operatorStack.top().precedence != OperatorPrecedence::PREFIX) {
        operatorStack.top().lineNumber = operandStack.top()->lineNumber();
        operatorStack.top().charIndex = operandStack.top()->charIndex();
      }

      for (int i = operatorStack.top().numOperands - 1; i >= 0; --i) {
        operands[i] = std::move(operandStack.top());
        operandStack.pop();
      }

      operandStack.push(std::make_unique<Expression>(context, operatorStack.top().operation, std::move(operands), operatorStack.top().lineNumber, operatorStack.top().charIndex));

      operatorStack.pop();
    }

    Expression::Ptr parseExpression(CompilerContext& context, TokenIterator& it, bool allowComma, bool allowEmpty) {
      std::stack<Expression::Ptr> operandStack;
      std::stack<OperatorInfo> operatorStack;

      bool expectingOperand = true;
      for (; !isExpressionEnd(*it, allowComma); ++it) {
        if (it->isReservedToken()) {
          OperatorInfo oi = getOperatorInfo(it->getReservedToken(), expectingOperand, it->lineNumber(), it->charIndex());

          if (oi.operation == Operation::CALL && expectingOperand) {
            // Expression grouping (expr)
            ++it;
            operandStack.push(parseExpression(context, it, false, false));
            if (!it->hasValue(ReservedToken::ROUND_R))
              throw SyntaxError("could not find a matching ')'.", oi.lineNumber, oi.charIndex, 0);
            expectingOperand = false;
            continue;
          }

          if (oi.operation == Operation::SUBSCRIPT && expectingOperand) {
            // Array literal [item, ...]
            oi.operation = Operation::ARRAY;
            oi.precedence = OperatorPrecedence::CONTAINER;
            oi.numOperands = 0;
            ++it;
            if (it->hasValue(ReservedToken::SQUARE_R))
              break;
            while (true) {
              Expression::Ptr item = parseExpression(context, it, false, false);
              operandStack.push(std::move(item));
              ++oi.numOperands;
              if (it->hasValue(ReservedToken::SQUARE_R))
                break;
              if (!it->hasValue(ReservedToken::COMMA))
                throw SyntaxError("could not find a matching ']'.", oi.lineNumber, oi.charIndex, 0);
              ++it;
            }
            operatorStack.push(oi);
            expectingOperand = false;
            continue;
          }

          if ((oi.precedence == OperatorPrecedence::PREFIX) != expectingOperand) {
            throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), true);
          }

          if (!operatorStack.empty() && isEvaluatedBefore(operatorStack.top(), oi)) {
            popOperator(operatorStack, operandStack, context, it->lineNumber(), it->charIndex());
          }

          switch (oi.operation) {
            case Operation::CALL: {
              ++it;
              if (it->hasValue(ReservedToken::ROUND_R))
                break;
              while (true) {
                Expression::Ptr argument = parseExpression(context, it, false, false);
                operandStack.push(std::move(argument));
                ++oi.numOperands;
                if (it->hasValue(ReservedToken::ROUND_R))
                  break;
                if (!it->hasValue(ReservedToken::COMMA))
                  throw SyntaxError("could not find a matching ')'.", it->lineNumber(), it->charIndex(), 0);
                ++it;
              }
              break;
            }

            case Operation::SUBSCRIPT:
              ++it;
              operandStack.push(parseExpression(context, it, true, false));
              if (!it->hasValue(ReservedToken::SQUARE_R))
                throw SyntaxError("could not find a matching ']'.", it->lineNumber(), it->charIndex(), 0);
              break;

            case Operation::TERNARY:
              ++it;
              operandStack.push(parseExpression(context, it, false, false));
              if (!it->hasValue(ReservedToken::COLON))
                throw SyntaxError("expected ':' to complete ternary expression.", it->lineNumber(), it->charIndex(), 0);
              break;

            default:
              break;
          }

          operatorStack.push(oi);
          expectingOperand = oi.precedence != OperatorPrecedence::POSTFIX;

        } else {
          if (!expectingOperand)
            throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), false);

          ExpressionValue value;

          if (it->isNull())
            value = VoidValue{};

          else if (it->isByte())
            value = it->getByte();

          else if (it->isShort())
            value = it->getShort();

          else if (it->isInt())
            value = it->getInt();

          else if (it->isLong())
            value = it->getLong();

          else if (it->isFloat())
            value = it->getFloat();

          else if (it->isDouble())
            value = it->getDouble();

          else if (it->isBool())
            value = it->getBool();

          else if (it->isChar())
            value = it->getChar();

          else if (it->isStr())
            value = it->getStr();

          else if (it->isIdentifier())
            value = it->getIdentifier();

          operandStack.push(std::make_unique<Expression>(context, value, std::vector<Expression::Ptr>(), it->lineNumber(), it->charIndex()));
          expectingOperand = false;
        }
      }

      if (expectingOperand) {
        if (allowEmpty && operandStack.empty() && operatorStack.empty())
          return Expression::Ptr();
        throw SyntaxError("expected an operand.", it->lineNumber(), it->charIndex(), 0);
      }

      while (!operatorStack.empty()) {
        popOperator(operatorStack, operandStack, context, it->lineNumber(), it->charIndex());
      }

      if (operandStack.size() != 1 || !operatorStack.empty()) {
        std::string str = "failed to parse expression. (resolved to ";
        str += std::to_string(operandStack.size());
        str += " operands, ";
        str += std::to_string(operatorStack.size());
        throw CompileError(str + " operators.)", it->lineNumber(), it->charIndex(), 0);
      }

      return std::move(operandStack.top());
    }
    
  }

  Expression::Ptr generateParseTree(CompilerContext& context, TokenIterator& it, TypeHandle type, bool lvalue, bool allowComma, bool allowEmpty) {
    Expression::Ptr ret = parseExpression(context, it, allowComma, allowEmpty);
    ret->checkConversion(type, lvalue);
    return ret;
  }
}