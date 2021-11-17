#include "statement_manager.hpp"

#include <stack>

#include "errors.hpp"
#include "tokenizer.hpp"


namespace valley {

  namespace {

    TypeHandle getTypeFromKeyword(ReservedToken kw) {
      switch (kw) {
        case ReservedToken::TYPE_ANY:
          return TypeRegistry::anyHandle();

        case ReservedToken::TYPE_BOOL:
          return TypeRegistry::boolHandle();

        case ReservedToken::TYPE_BYTE:
          return TypeRegistry::byteHandle();

        case ReservedToken::TYPE_CHAR:
          return TypeRegistry::charHandle();

        case ReservedToken::TYPE_DOUBLE:
          return TypeRegistry::doubleHandle();

        case ReservedToken::TYPE_FLOAT:
          return TypeRegistry::floatHandle();

        case ReservedToken::TYPE_INT:
          return TypeRegistry::intHandle();

        case ReservedToken::TYPE_LONG:
          return TypeRegistry::longHandle();

        case ReservedToken::TYPE_SHORT:
          return TypeRegistry::shortHandle();

        case ReservedToken::TYPE_STR:
          return TypeRegistry::strHandle();

        case ReservedToken::TYPE_VOID:
          return TypeRegistry::voidHandle();

        default:
          return nullptr;
      }
    }

  }

  Statement::Ptr parseDeclaration(CompilerContext& context, TokenIterator& it, Statement::Ptr parent, TypeHandle type, bool isFinal, bool isStatic, size_t lineNumber, size_t charIndex) {
    if (it->isReservedToken()) {
      if (it->getReservedToken() == ReservedToken::KW_FINAL) {
        if (isFinal)
          throw SyntaxError("variable is already specified as final.", it->lineNumber(), it->charIndex(), 5);
        return parseDeclaration(context, ++it, parent, type, true, isStatic, lineNumber, charIndex);
      }
      if (it->getReservedToken() == ReservedToken::KW_STATIC) {
        if (isStatic)
          throw SyntaxError("variable is already specified as static.", it->lineNumber(), it->charIndex(), 6);
        return parseDeclaration(context, ++it, parent, type, isFinal, true, lineNumber, charIndex);
      }
      if (it->getReservedToken() == ReservedToken::SQUARE_L) {
        if (!type)
          throw SyntaxError("encountered '[]' before type in declaration.", it->lineNumber(), it->charIndex(), 2);
        ++it;
        if (it->hasValue(ReservedToken::SQUARE_R)) {
          ArrayType at;
          at.inner = type;
          return parseDeclaration(context, ++it, parent, context.getHandle(at), isFinal, isStatic, lineNumber, charIndex);
        }
      }
      if (TypeHandle t = getTypeFromKeyword(it->getReservedToken())) {
        if (type)
          throw SyntaxError("variable has already been specified as type '" + typeHandleRepr(type) + "'.", it->lineNumber(), it->charIndex(), typeHandleRepr(type).length());
        return parseDeclaration(context, ++it, parent, t, isFinal, isStatic, lineNumber, charIndex);
      }
      throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), false);
    } else if (it->isIdentifier()) {
      if (!type)
        throw SyntaxError("missing type name in variable declaration.", it->lineNumber(), it->charIndex(), 0);
      const std::string name = it->getIdentifier().name;
      if (context.find(name))
        throw SemanticError("variable '" + name + "' already exists in the current scope.", it->lineNumber(), it->charIndex(), name.length());
      ++it;
      if (it->isReservedToken() && it->getReservedToken() == ReservedToken::ROUND_L) {
        // Function declaration
        std::vector<IdentifierInfo> paramInfos;
        std::vector<std::string> paramNames;
        context.enterFunction();
        FuncType ft;
        ft.returnType = type;
        while (!it->hasValue(ReservedToken::ROUND_R)) {
          ++it;
          if (!it->isReservedToken())
            throw SyntaxError("expected a parameter type name.", it->lineNumber(), it->charIndex(), 0);
          TypeHandle paramType = getTypeFromKeyword(it->getReservedToken());
          if (!paramType)
            throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), false);
          ++it;
          while (true) {
            if (it->hasValue(ReservedToken::SQUARE_L)) {
              // Nest type into an array []
              ++it;
              if (!it->hasValue(ReservedToken::SQUARE_R))
                throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), false);
              ArrayType at;
              at.inner = paramType;
              paramType = context.getHandle(at);
              ++it;
              continue;
            } else if (it->hasValue(ReservedToken::ELLIPSIS)) {
              // ... Make into arg catcher (only one allowed per signature)
              if (ft.hasArgCatch)
                throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), false);
              ft.hasArgCatch = true;
              ArrayType at;
              at.inner = paramType;
              paramType = context.getHandle(at);
              ++it;
              continue;
            } else if (it->isIdentifier()) {
              const std::string paramName = it->getIdentifier().name;
              IdentifierInfo paramInfo = *context.createParam(paramName, paramType);
              paramInfos.push_back(paramInfo);
              paramNames.push_back(paramName);
              ft.paramTypes.push_back(paramType);
              ++it;
              break;
            }
            throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), 1);
          }
          if (!it->hasValue(ReservedToken::COMMA) && !it->hasValue(ReservedToken::ROUND_R))
            throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), true);
        }
        ++it;
        Statement::Ptr exec;
        if (it->hasValue(ReservedToken::SEMICOLON))
          exec = std::make_shared<StatementEmpty>(parent, lineNumber, charIndex);
        else if (it->hasValue(ReservedToken::CURLY_L))
          exec = parseStatement(context, it, nullptr, false, true, false, false, false, true, false);
        else
          throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), false);
        context.leaveScope();
        const IdentifierInfo* info = context.createIdentifier(name, context.getHandle(ft), isFinal, isStatic);
        Statement::Ptr stmt = std::make_shared<StatementDecfunc>(parent, info, std::move(name), paramInfos, paramNames, exec, lineNumber, charIndex);
        exec->setParent(stmt);
        return stmt;
      }
      // Variable declaration
      const IdentifierInfo* info = context.createIdentifier(name, type, isFinal, isStatic);
      Statement::Ptr value;
      if (it->hasValue(ReservedToken::EQUAL))
        value = parseStatement(context, ++it, nullptr, false, false, false, false, false, true, true);
      else
        value = std::make_shared<StatementEmpty>(nullptr, it->lineNumber(), it->charIndex());
      if (!it->hasValue(ReservedToken::SEMICOLON) && !it->hasValue(ReservedToken::COLON))
        throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), true);
      Statement::Ptr stmt = std::make_shared<StatementDeclare>(parent, info, std::move(name), value, lineNumber, charIndex);
      value->setParent(stmt);
      return stmt;
    }
    throw SyntaxError_unexpected(it->toString(), it->lineNumber(), it->charIndex(), true);
  }

  Statement::Ptr parseStatement(CompilerContext& context, TokenIterator& it, Statement::Ptr parent, bool allowEmpty, bool allowReturn, bool allowBreak, bool allowContinue, bool allowSwitchCase, bool allowDeclare, bool requireEvalValue) {
    size_t lineNumber = it->lineNumber();
    size_t charIndex = it->charIndex();

    Statement::Ptr parsed;
    bool isExpr = false;

    if (it->isReservedToken()) {
      switch (it->getReservedToken()) {
        // StatementEmpty
        case ReservedToken::SEMICOLON: {
          if (!allowEmpty)
            throw SyntaxError("expected a statement at this position.", lineNumber, charIndex, 0);
          parsed = std::make_shared<StatementEmpty>(parent, lineNumber, charIndex);
          break;
        }
        
        // StatementBlock
        // {...}
        case ReservedToken::CURLY_L: {
          if (requireEvalValue)
            throw SyntaxError("expected an evaluable statement, got code block instead.", lineNumber, charIndex, 1);
          ++it;
          std::vector<Statement::Ptr> contents;
          for (; !it->hasValue(ReservedToken::CURLY_R); ++it) {
            if (it->isEof())
              throw SyntaxError("could not find a matching '}'.", lineNumber, charIndex, 0);
            Statement::Ptr stmt = parseStatement(context, it, nullptr, true, allowReturn, allowBreak, allowContinue, false, true, false);
            if (stmt->type() != StatementType::EMPTY)
              contents.push_back(stmt);
          }
          parsed = std::make_shared<StatementBlock>(parent, contents, lineNumber, charIndex);
          // std::vector<Statement::Ptr>& contents_ref = parsed_stmt->get_contents();
          // for (size_t i = 0; i < contents_ref.size(); i++) {
          //   contents_ref[i]->set_parent(parsed_stmt);
          // }
          break;
        }

        // StatementReturn
        // return [...];
        case ReservedToken::KW_RETURN: {
          if (!allowReturn)
            throw SemanticError("encountered 'return' outside function definition.", lineNumber, charIndex, 6);
          if (requireEvalValue)
            throw SyntaxError("expected an evaluable statement, got 'return' instead.", lineNumber, charIndex, 6);
          ++it;
          Statement::Ptr stmt = parseStatement(context, it, nullptr, true, false, false, false, false, true, true);
          parsed = std::make_shared<StatementReturn>(parent, stmt, lineNumber, charIndex);
          stmt->setParent(parsed);
          break;
        }

        // StatementBreak
        // break;
        case ReservedToken::KW_BREAK: {
          if (!allowBreak)
            throw SemanticError("encountered 'break' outside loop/switch.", lineNumber, charIndex, 5);
          if (requireEvalValue)
            throw SyntaxError("expected an evaluable statement, got 'break' instead.", lineNumber, charIndex, 5);
          ++it;
          if (!it->hasValue(ReservedToken::SEMICOLON))
            throw SyntaxError("unexpected statement within 'break'.", it->lineNumber(), it->charIndex(), 0);
          parsed = std::make_shared<StatementBreak>(parent, lineNumber, charIndex);
          break; // ironic...
        }

        // StatementContinue
        // continue;
        case ReservedToken::KW_CONTINUE: {
          if (!allowContinue)
            throw SemanticError("encountered 'continue' outside loop.", lineNumber, charIndex, 8);
          if (requireEvalValue)
            throw SyntaxError("expected an evaluable statement, got 'continue' instead.", lineNumber, charIndex, 8);
          ++it;
          if (!it->hasValue(ReservedToken::SEMICOLON))
            throw SyntaxError("unexpected statement within 'continue'.", it->lineNumber(), it->charIndex(), 0);
          parsed = std::make_shared<StatementContinue>(parent, lineNumber, charIndex);
          break;
        }

        // StatementIfElse
        // if (...) ...; [else ...;]
        case ReservedToken::KW_IF: {
          if (requireEvalValue)
            throw SyntaxError("expected an evaluable statement, got 'if' instead.", lineNumber, charIndex, 2);
          ++it;
          if (!it->hasValue(ReservedToken::ROUND_L))
            throw SyntaxError("expected opening '(' after 'if' keyword.", it->lineNumber(), it->charIndex(), 0);
          ++it;
          Statement::Ptr ifCondition = parseStatement(context, it, nullptr, false, false, false, false, false, true, true);
          if (!it->hasValue(ReservedToken::ROUND_R))
            throw SyntaxError("expected closing ')' after 'if' condition.", it->lineNumber(), it->charIndex(), 0);
          ++it;
          if (it->hasValue(ReservedToken::KW_ELSE))
            throw SyntaxError("expected statement between 'if' and 'else' clauses.", it->lineNumber(), it->charIndex(), 0);
          Statement::Ptr doIf = parseStatement(context, it, nullptr, false, allowReturn, allowBreak, allowContinue, false, true, false);
          Token savedToken = *it;
          ++it;
          Statement::Ptr doElse;
          if (it->hasValue(ReservedToken::KW_ELSE))
            doElse = parseStatement(context, ++it, nullptr, false, allowReturn, allowBreak, allowContinue, false, true, false);
          else {
            doElse = std::make_shared<StatementEmpty>(nullptr, it->lineNumber(), it->charIndex());
            // Hacky method of stepping back if there's no else statement
            it.stepBack(savedToken);
          }
          parsed = std::make_shared<StatementIfElse>(parent, ifCondition, doIf, doElse, lineNumber, charIndex);
          ifCondition->setParent(parsed);
          doIf->setParent(parsed);
          doElse->setParent(parsed);
          break;
        }

        case ReservedToken::KW_ELIF: // Should never be encountered, as "elif" is internally converted to "else if" when tokenizing
        case ReservedToken::KW_ELSE:
          throw SyntaxError("encountered 'else' without supporting 'if' statement.", lineNumber, charIndex, 4);
        
        // StatementWhile
        // while (...) ...;
        case ReservedToken::KW_WHILE: {
          if (requireEvalValue)
            throw SyntaxError("expected an evaluable statement, got 'while' instead.", lineNumber, charIndex, 5);
          ++it;
          if (!it->hasValue(ReservedToken::ROUND_L))
            throw SyntaxError("expected opening '(' after 'while' keyword.", it->lineNumber(), it->charIndex(), 0);
          ++it;
          Statement::Ptr whileCondition = parseStatement(context, it, nullptr, false, false, false, false, false, true, true);
          if (!it->hasValue(ReservedToken::ROUND_R))
            throw SyntaxError("expected closing ')' after while loop declaration.", it->lineNumber(), it->charIndex(), 0);
          ++it;
          Statement::Ptr whileLooped = parseStatement(context, it, nullptr, true, allowReturn, true, true, false, true, false);
          parsed = std::make_shared<StatementWhile>(parent, whileCondition, whileLooped, lineNumber, charIndex);
          whileCondition->setParent(parsed);
          whileLooped->setParent(parsed);
          break;
        }
        
        // StatementDoWhile
        // do ... while (...);
        case ReservedToken::KW_DO: {
          if (requireEvalValue)
            throw SemanticError("expected an evaluable statement, got 'do' instead.", lineNumber, charIndex, 2);
          ++it;
          Statement::Ptr doWhileLooped = parseStatement(context, it, nullptr, false, allowReturn, true, true, false, true, false);
          if (!it->hasValue(ReservedToken::KW_WHILE))
            throw SyntaxError("expected 'while' keyword following 'do' statement.", it->lineNumber(), it->charIndex(), 0);
          ++it;
          if (!it->hasValue(ReservedToken::ROUND_L))
            throw SyntaxError("expected opening '(' after 'do' keyword.", it->lineNumber(), it->charIndex(), 0);
          ++it;
          Statement::Ptr doWhileCondition = parseStatement(context, it, nullptr, false, false, false, false, false, true, true);
          if (!it->hasValue(ReservedToken::ROUND_R))
            throw SyntaxError("expected closing ')' after loop condition.", it->lineNumber(), it->charIndex(), 0);
          parsed = std::make_shared<StatementDoWhile>(parent, doWhileCondition, doWhileLooped, lineNumber, charIndex);
          doWhileCondition->setParent(parsed);
          doWhileLooped->setParent(parsed);
          break;
        }
        
        // StatementFor, StatementForeach
        // for ([...]; [...]; [...]) ...;
        // for (... : ...) ...;
        case ReservedToken::KW_FOR: {
          if (requireEvalValue)
            throw SyntaxError("expected an evaluable statement, got 'for' instead.", lineNumber, charIndex, 3);
          ++it;
          if (!it->hasValue(ReservedToken::ROUND_L))
            throw SyntaxError("expected opening '(' after 'for' keyword.", it->lineNumber(), it->charIndex(), 0);
          ++it;

          // Get first statement in declaration (for: init, for-each: iterator)
          Statement::Ptr forFirst = parseStatement(context, it, nullptr, true, false, false, false, false, true, false);
          // Determine from end of first statement whether to build a for (;) or foreach (:) loop
          bool isForeach;
          if (it->hasValue(ReservedToken::SEMICOLON))
            isForeach = false;
          else if (it->hasValue(ReservedToken::COLON))
            isForeach = true;
          else
            throw SyntaxError("expected ';' or ':', got '" + it->toString() + "' instead.", it->lineNumber(), it->charIndex(), 0);

          // Check according to whether loop is for or foreach
          if (isForeach && forFirst->type() == StatementType::EMPTY)
            throw SyntaxError("expected a declaration statement before ':' in for-each loop declaration.", it->lineNumber(), it->charIndex(), 1);

          if (isForeach) {
            // Foreach loop (:)
            if (forFirst->type() != StatementType::DECLARE)
              throw SyntaxError("an iterator variable must be declared before ':'.", it->lineNumber(), it->charIndex(), 1);
            ++it;
            Statement::Ptr forIter = parseStatement(context, it, nullptr, false, false, false, false, false, false, true);
            if (!it->hasValue(ReservedToken::ROUND_R))
              throw SyntaxError("expected closing ')' after for-each loop declaration.", it->lineNumber(), it->charIndex(), 0);
            ++it;
            Statement::Ptr forLooped = parseStatement(context, it, nullptr, true, allowReturn, true, true, false, true, false);
            parsed = std::make_shared<StatementForeach>(parent, forFirst, forIter, forLooped, lineNumber, charIndex);
            forFirst->setParent(parsed);
            forIter->setParent(parsed);
            forLooped->setParent(parsed);

          } else {
            // Regular for loop (;)
            ++it;
            Statement::Ptr forCondition = parseStatement(context, it, nullptr, false, false, false, false, false, false, true);
            if (!it->hasValue(ReservedToken::SEMICOLON))
              throw SyntaxError("expected ';', got '" + it->toString() + "' instead.", it->lineNumber(), it->charIndex(), 0);
            ++it;
            Statement::Ptr forOnIter = parseStatement(context, it, nullptr, true, allowReturn, true, true, false, true, false);
            if (!it->hasValue(ReservedToken::ROUND_R))
              throw SyntaxError("expected closing ')' after for loop declaration.", it->lineNumber(), it->charIndex(), 0);
            ++it;
            Statement::Ptr forLooped = parseStatement(context, it, nullptr, true, allowReturn, true, true, false, true, false);
            parsed = std::make_shared<StatementFor>(parent, forFirst, forCondition, forOnIter, forLooped, lineNumber, charIndex);
            forFirst->setParent(parsed);
            forCondition->setParent(parsed);
            forOnIter->setParent(parsed);
            forLooped->setParent(parsed);
          }
          break;
        }

        // StatementDeclare, StatementDecfunc
        case ReservedToken::KW_FINAL:
          parsed = parseDeclaration(context, ++it, parent, nullptr, true, false, lineNumber, charIndex);
          break;
        case ReservedToken::KW_STATIC:
          parsed = parseDeclaration(context, ++it, parent, nullptr, false, true, lineNumber, charIndex);
          break;
        case ReservedToken::TYPE_ANY:
        case ReservedToken::TYPE_BOOL:
        case ReservedToken::TYPE_BYTE:
        case ReservedToken::TYPE_CHAR:
        case ReservedToken::TYPE_DOUBLE:
        case ReservedToken::TYPE_FLOAT:
        case ReservedToken::TYPE_INT:
        case ReservedToken::TYPE_LONG:
        case ReservedToken::TYPE_SHORT:
        case ReservedToken::TYPE_STR:
        case ReservedToken::TYPE_VOID: {
          TypeHandle type = getTypeFromKeyword(it->getReservedToken());
          parsed = parseDeclaration(context, ++it, parent, type, false, false, lineNumber, charIndex);
          break;
        }

        // StatementExpr
        default:
          isExpr = true;
          break;
      }
    } else {
      // Treat the statement as an expression
      isExpr = true;
    }

    if (isExpr) {
      Expression::Ptr expr = generateParseTree(context, it, TypeRegistry::voidHandle(), false, false, false);
      parsed = std::make_shared<StatementExpr>(parent, expr, lineNumber, charIndex);
    }

    // Failsafe to prevent potential issues with assuming that the statement is present
    if (!parsed || (!allowEmpty && parsed->type() == StatementType::EMPTY))
      throw SyntaxError("expected a statement at this position.", lineNumber, charIndex, 0);

    return parsed;
  }

  std::vector<Statement::Ptr> parseCode(CompilerContext& context, TokenIterator& it) {
    std::vector<Statement::Ptr> code;
    for (; !it->isEof(); ++it) {
      Statement::Ptr parsed = parseStatement(context, it, nullptr, true, false, false, false, false, true, false);
      if (parsed->type() != StatementType::EMPTY)
        code.push_back(parsed);
    }
    return std::move(code);
  }
  
}