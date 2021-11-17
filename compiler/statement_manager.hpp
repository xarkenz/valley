#pragma once

#include "statement.hpp"


namespace valley {

  Statement::Ptr parseDeclaration(CompilerContext& context, TokenIterator& it, Statement::Ptr parent, TypeHandle type, bool isFinal, bool isStatic, size_t lineNumber, size_t charIndex);

  Statement::Ptr parseStatement(CompilerContext& context, TokenIterator& it, Statement::Ptr parent, bool allowEmpty, bool allowReturn, bool allowBreak, bool allowContinue, bool allowSwitchCase, bool allowDeclare, bool requireEvalValue);
  
  std::vector<Statement::Ptr> parseCode(CompilerContext& context, TokenIterator& it);

}