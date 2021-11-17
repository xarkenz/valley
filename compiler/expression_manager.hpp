#pragma once

#include "expression.hpp"
#include "types.hpp"


namespace valley {

  class TokenIterator;
  class CompilerContext;

  Expression::Ptr generateParseTree(CompilerContext& context, TokenIterator& it, TypeHandle type, bool lvalue, bool allowComma, bool allowEmpty);
  
}