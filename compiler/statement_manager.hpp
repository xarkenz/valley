#ifndef STATEMENT_MANAGER_HPP
#define STATEMENT_MANAGER_HPP

#include <memory>
#include <iostream>

#include "statement.hpp"

namespace valley {
  statement::ptr parse_declare(compiler_context& context, tokens_iterator& it, statement::ptr parent, type_handle type_id, bool is_final, bool is_static, size_t line_number, size_t char_index);

  statement::ptr parse_statement(compiler_context& context, tokens_iterator& it, statement::ptr parent, bool allow_empty, bool allow_return, bool allow_break, bool allow_continue, bool allow_switch_case, bool allow_declare, bool require_eval_value);
  
  std::vector<statement::ptr>& parse_code_statements(compiler_context& context, tokens_iterator& it, std::vector<statement::ptr>& parsed_code);
}

#endif
