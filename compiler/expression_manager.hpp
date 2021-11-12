#ifndef EXPRESSION_MANAGER_HPP
#define EXPRESSION_MANAGER_HPP

#include <memory>

#include "expression.hpp"
#include "types.hpp"

namespace valley {
  class node;
  using node_ptr = std::unique_ptr<node>;
  class tokens_iterator;
  class compiler_context;

  node_ptr parse_expression_tree(compiler_context& context, tokens_iterator& it, type_handle type_id, bool lvalue, bool allow_comma, bool allow_empty);
}

#endif
