#include "statement_manager.hpp"

#include <stack>
#include <iostream>

#include "errors.hpp"
#include "tokenizer.hpp"

namespace valley {

  namespace {
    type_handle get_typekw_handle(reserved_token typekw) {
      switch (typekw) {
        case reserved_token::typekw_bool:
          return type_registry::bool_handle();
        case reserved_token::typekw_byte:
          return type_registry::byte_handle();
        case reserved_token::typekw_char:
          return type_registry::char_handle();
        case reserved_token::typekw_double:
          return type_registry::double_handle();
        case reserved_token::typekw_float:
          return type_registry::float_handle();
        case reserved_token::typekw_int:
          return type_registry::int_handle();
        case reserved_token::typekw_long:
          return type_registry::long_handle();
        case reserved_token::typekw_short:
          return type_registry::short_handle();
        case reserved_token::typekw_str:
          return type_registry::str_handle();
        case reserved_token::typekw_void:
          return type_registry::void_handle();
        default:
          return nullptr;
      }
    }
  }

  statement::ptr parse_declare(compiler_context& context, tokens_iterator& it, statement::ptr parent, type_handle type_id, bool is_final, bool is_static, size_t line_number, size_t char_index) {
    if (it->is_reserved_token()) {
      if (it->get_reserved_token() == reserved_token::kw_final) {
        if (is_final)
          throw syntax_error("name is already specified as final.", it->get_line_number(), it->get_char_index(), 5);
        return parse_declare(context, ++it, parent, type_id, true, is_static, line_number, char_index);
      }
      if (it->get_reserved_token() == reserved_token::kw_static) {
        if (is_static)
          throw syntax_error("name is already specified as static.", it->get_line_number(), it->get_char_index(), 6);
        return parse_declare(context, ++it, parent, type_id, is_final, true, line_number, char_index);
      }
      if (it->get_reserved_token() == reserved_token::lbrckt) {
        if (!type_id)
          throw syntax_error("encountered '[]' before name type.", it->get_line_number(), it->get_char_index(), 2);
        ++it;
        if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::rbrckt) {
          array_type at;
          at.inner = type_id;
          return parse_declare(context, ++it, parent, context.get_handle(at), is_final, is_static, line_number, char_index);
        }
      }
      if (type_handle t = get_typekw_handle(it->get_reserved_token())) {
        if (type_id)
          throw syntax_error("name has already been specified as type '" + std::to_string(type_id) + "'.", it->get_line_number(), it->get_char_index(), std::to_string(type_id).length());
        return parse_declare(context, ++it, parent, t, is_final, is_static, line_number, char_index);
      }
      throw unexpected_syntax_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), false);
    } else if (it->is_identifier()) {
      if (!type_id)
        throw syntax_error("missing type name in variable declaration.", it->get_line_number(), it->get_char_index(), 0);
      const std::string id_name = it->get_identifier().name;
      if (context.find(id_name))
        throw semantic_error("variable '" + id_name + "' already exists in the current scope.", it->get_line_number(), it->get_char_index(), id_name.size());
      ++it;
      if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::lparen) {
        // Function declaration
        std::vector<identifier_info> param_infos;
        std::vector<std::string> param_names;
        context.enter_function();
        function_type ft;
        ft.returning = type_id;
        while (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::rparen) {
          ++it;
          if (!it->is_reserved_token())
            throw syntax_error("expected a parameter type name.", it->get_line_number(), it->get_char_index(), 0);
          type_handle param_type = get_typekw_handle(it->get_reserved_token());
          if (!param_type)
            throw unexpected_syntax_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), false);
          ++it;
          while (true) {
            if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::lbrckt) {
              // [] Nest type into an array
              ++it;
              if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::rbrckt)
                throw unexpected_syntax_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), false);
              array_type at;
              at.inner = param_type;
              param_type = context.get_handle(at);
              ++it;
              continue;
            } else if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::ellipsis) {
              // ... Make into arg catcher (only one allowed per signature)
              if (ft.has_varargs)
                throw unexpected_syntax_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), false);
              ft.has_varargs = true;
              array_type at;
              at.inner = param_type;
              param_type = context.get_handle(at);
              ++it;
              continue;
            } else if (it->is_identifier()) {
              const std::string param_name = it->get_identifier().name;
              identifier_info param_info = *context.create_param(param_name, param_type);
              param_infos.push_back(param_info);
              param_names.push_back(param_name);
              ft.params.push_back(param_type);
              ++it;
              break;
            }
            throw unexpected_syntax_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), 1);
          }
          if (!it->is_reserved_token() || (it->get_reserved_token() != reserved_token::rparen && it->get_reserved_token() != reserved_token::comma))
            throw unexpected_syntax_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), true);
        }
        ++it;
        statement::ptr exec;
        if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::semic)
          exec = std::make_shared<statement_empty>(parent, line_number, char_index);
        else if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::lbrace)
          exec = parse_statement(context, it, nullptr, false, true, false, false, false, true, false);
        else
          throw unexpected_syntax_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), false);
        context.leave_scope();
        const identifier_info* id_info = context.create_identifier(id_name, context.get_handle(ft), is_final, is_static);
        statement::ptr stmt = std::make_shared<statement_decfunc>(parent, id_info, std::move(id_name), param_infos, param_names, exec, line_number, char_index);
        exec->set_parent(stmt);
        return stmt;
      }
      // Variable declaration
      const identifier_info* id_info = context.create_identifier(id_name, type_id, is_final, is_static);
      statement::ptr value;
      if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::equal)
        value = parse_statement(context, ++it, nullptr, false, false, false, false, false, true, true);
      else
        value = std::make_shared<statement_empty>(nullptr, it->get_line_number(), it->get_char_index());
      if (!it->is_reserved_token() || (it->get_reserved_token() != reserved_token::semic && it->get_reserved_token() != reserved_token::colon))
        throw unexpected_syntax_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), true);
      statement::ptr stmt = std::make_shared<statement_declare>(parent, id_info, std::move(id_name), value, line_number, char_index);
      value->set_parent(stmt);
      return stmt;
    }
    throw unexpected_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), true);
  }

  statement::ptr parse_statement(compiler_context& context, tokens_iterator& it, statement::ptr parent, bool allow_empty, bool allow_return, bool allow_break, bool allow_continue, bool allow_switch_case, bool allow_declare, bool require_eval_value) {
    size_t line_number = it->get_line_number();
    size_t char_index = it->get_char_index();

    statement::ptr parsed_stmt;
    bool is_expr = false;

    if (it->is_reserved_token()) {
      switch (it->get_reserved_token()) {
        // Statement is empty
        case reserved_token::semic: {
          if (!allow_empty)
            throw semantic_error("expected a statement at this position.", line_number, char_index, 0);
          parsed_stmt = std::make_shared<statement_empty>(parent, line_number, char_index);
          break;
        }
        
        // Statement is a block of code
        // {...}
        case reserved_token::lbrace: {
          if (require_eval_value)
            throw semantic_error("expected an evaluable statement, got code block instead.", line_number, char_index, 1);
          ++it;
          std::vector<statement::ptr> contents;
          for (; !it->is_reserved_token() || it->get_reserved_token() != reserved_token::rbrace; ++it) {
            if (it->is_eof())
              throw syntax_error("could not find a matching '}'.", line_number, char_index, 0);
            statement::ptr stmt = parse_statement(context, it, nullptr, true, allow_return, allow_break, allow_continue, false, true, false);
            if (stmt->get_stmt_type() != statement_type::empty_s)
              contents.push_back(stmt);
          }
          parsed_stmt = std::make_shared<statement_block>(parent, contents, line_number, char_index);
          // std::vector<statement::ptr>& contents_ref = parsed_stmt->get_contents();
          // for (size_t i = 0; i < contents_ref.size(); i++) {
          //   contents_ref[i]->set_parent(parsed_stmt);
          // }
          break;
        }

        // Statement returns value
        // return [...];
        case reserved_token::kw_return: {
          if (!allow_return)
            throw semantic_error("encountered 'return' outside function definition.", line_number, char_index, 6);
          if (require_eval_value)
            throw semantic_error("expected an evaluable statement, got 'return' instead.", line_number, char_index, 6);
          ++it;
          statement::ptr stmt = parse_statement(context, it, nullptr, true, false, false, false, false, true, true);
          parsed_stmt = std::make_shared<statement_return>(parent, stmt, line_number, char_index);
          stmt->set_parent(parsed_stmt);
          break;
        }

        // Statement breaks flow
        // break;
        case reserved_token::kw_break: {
          if (!allow_break)
            throw semantic_error("encountered 'break' outside loop/switch.", line_number, char_index, 5);
          if (require_eval_value)
            throw semantic_error("expected an evaluable statement, got 'break' instead.", line_number, char_index, 5);
          ++it;
          if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::semic)
            throw semantic_error("unexpected statement within 'break'.", it->get_line_number(), it->get_char_index(), 0);
          parsed_stmt = std::make_shared<statement_break>(parent, line_number, char_index);
          break; // ironic.
        }

        // Statement continues loop
        // continue;
        case reserved_token::kw_continue: {
          if (!allow_continue)
            throw semantic_error("encountered 'continue' outside loop.", line_number, char_index, 8);
          if (require_eval_value)
            throw semantic_error("expected an evaluable statement, got 'continue' instead.", line_number, char_index, 8);
          ++it;
          if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::semic)
            throw semantic_error("unexpected statement within 'continue'.", it->get_line_number(), it->get_char_index(), 0);
          parsed_stmt = std::make_shared<statement_continue>(parent, line_number, char_index);
          break;
        }

        // Statement contains if-else
        // if (...) ...; [else ...;]
        case reserved_token::kw_if: {
          if (require_eval_value)
            throw semantic_error("expected an evaluable statement, got 'if' instead.", line_number, char_index, 2);
          ++it;
          if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::lparen)
            throw syntax_error("expected opening '(' after 'if' keyword.", it->get_line_number(), it->get_char_index(), 0);
          ++it;
          statement::ptr if_condition = parse_statement(context, it, nullptr, false, false, false, false, false, true, true);
          if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::rparen)
            throw syntax_error("expected closing ')' after 'if' condition.", it->get_line_number(), it->get_char_index(), 0);
          ++it;
          if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::kw_else)
            throw syntax_error("expected statement between 'if' and 'else' clauses.", it->get_line_number(), it->get_char_index(), 0);
          statement::ptr do_if = parse_statement(context, it, nullptr, false, allow_return, allow_break, allow_continue, false, true, false);
          token saved_token = *it;
          ++it;
          statement::ptr do_else;
          if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::kw_else)
            do_else = parse_statement(context, ++it, nullptr, false, allow_return, allow_break, allow_continue, false, true, false);
          else {
            do_else = std::make_shared<statement_empty>(nullptr, it->get_line_number(), it->get_char_index());
            // A bit hacky, but for all intents and purposes, it should work
            it.move_back(saved_token);
          }
          parsed_stmt = std::make_shared<statement_if_else>(parent, if_condition, do_if, do_else, line_number, char_index);
          if_condition->set_parent(parsed_stmt);
          do_if->set_parent(parsed_stmt);
          do_else->set_parent(parsed_stmt);
          break;
        }
        case reserved_token::kw_elif: // Should never be encountered, as "elif" is internally converted to "else if" when tokenizing
        case reserved_token::kw_else: {
          throw semantic_error("encountered 'else' without supporting 'if' statement.", line_number, char_index, 4);
        }
        
        // Statement contains while loop
        // while (...) ...;
        case reserved_token::kw_while: {
          if (require_eval_value)
            throw semantic_error("expected an evaluable statement, got 'while' instead.", line_number, char_index, 5);
          ++it;
          if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::lparen)
            throw syntax_error("expected opening '(' after 'while' keyword.", it->get_line_number(), it->get_char_index(), 0);
          ++it;
          statement::ptr while_condition = parse_statement(context, it, nullptr, false, false, false, false, false, true, true);
          if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::rparen)
            throw syntax_error("expected closing ')' after while loop declaration.", it->get_line_number(), it->get_char_index(), 0);
          ++it;
          statement::ptr while_looped = parse_statement(context, it, nullptr, true, allow_return, true, true, false, true, false);
          parsed_stmt = std::make_shared<statement_while>(parent, while_condition, while_looped, line_number, char_index);
          while_condition->set_parent(parsed_stmt);
          while_looped->set_parent(parsed_stmt);
          break;
        }
        
        // Statement contains do-while loop
        // do ... while (...);
        case reserved_token::kw_do: {
          if (require_eval_value)
            throw semantic_error("expected an evaluable statement, got 'do' instead.", line_number, char_index, 2);
          ++it;
          statement::ptr do_while_looped = parse_statement(context, it, nullptr, false, allow_return, true, true, false, true, false);
          if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::kw_while)
            throw syntax_error("expected 'while' keyword following 'do' clause.", it->get_line_number(), it->get_char_index(), 0);
          ++it;
          if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::lparen)
            throw syntax_error("expected opening '(' after 'do' keyword.", it->get_line_number(), it->get_char_index(), 0);
          ++it;
          statement::ptr do_while_condition = parse_statement(context, it, nullptr, false, false, false, false, false, true, true);
          if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::rparen)
            throw syntax_error("expected closing ')' after do-while loop declaration.", it->get_line_number(), it->get_char_index(), 0);
          parsed_stmt = std::make_shared<statement_do_while>(parent, do_while_condition, do_while_looped, line_number, char_index);
          do_while_condition->set_parent(parsed_stmt);
          do_while_looped->set_parent(parsed_stmt);
          break;
        }
        
        // Statement contains for/for-each loop
        // for ([...]; [...]; [...]) ...;
        // for (... : ...) ...;
        case reserved_token::kw_for: {
          if (require_eval_value)
            throw semantic_error("expected an evaluable statement, got 'for' instead.", line_number, char_index, 3);
          ++it;
          if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::lparen)
            throw syntax_error("expected opening '(' after 'for' keyword.", it->get_line_number(), it->get_char_index(), 0);
          ++it;
          // Get first statement in declaration (for: init, for-each: iterator)
          statement::ptr for_first = parse_statement(context, it, nullptr, true, false, false, false, false, true, false);
          // Determine from end of first statement whether to build a for (;) or for-each (:) loop
          bool is_foreach;
          if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::semic)
            is_foreach = false;
          else if (it->is_reserved_token() && it->get_reserved_token() == reserved_token::colon)
            is_foreach = true;
          else
            throw syntax_error("expected ';' or ':', got '" + std::to_string(*it) + "' instead.", it->get_line_number(), it->get_char_index(), 0);
          // Check according to whether loop is for or for-each
          if (is_foreach && for_first->get_stmt_type() == statement_type::empty_s)
            throw syntax_error("expected a declaration statement before ':' in for-each loop declaration.", it->get_line_number(), it->get_char_index(), 1);
          if (is_foreach) {
            if (for_first->get_stmt_type() != statement_type::declare_s)
              throw syntax_error("an iterator variable must be declared before ':'.", it->get_line_number(), it->get_char_index(), 1);
            ++it;
            statement::ptr for_iter = parse_statement(context, it, nullptr, false, false, false, false, false, false, true);
            if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::rparen)
              throw syntax_error("expected closing ')' after for-each loop declaration.", it->get_line_number(), it->get_char_index(), 0);
            ++it;
            statement::ptr for_looped = parse_statement(context, it, nullptr, true, allow_return, true, true, false, true, false);
            parsed_stmt = std::make_shared<statement_foreach>(parent, for_first, for_iter, for_looped, line_number, char_index);
            for_first->set_parent(parsed_stmt);
            for_iter->set_parent(parsed_stmt);
            for_looped->set_parent(parsed_stmt);
          } else {
            ++it;
            statement::ptr for_condition = parse_statement(context, it, nullptr, false, false, false, false, false, false, true);
            if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::semic)
              throw syntax_error("expected ';', got '" + std::to_string(*it) + "' instead.", it->get_line_number(), it->get_char_index(), 0);
            ++it;
            statement::ptr for_on_iter = parse_statement(context, it, nullptr, true, allow_return, true, true, false, true, false);
            if (!it->is_reserved_token() || it->get_reserved_token() != reserved_token::rparen)
              throw syntax_error("expected closing ')' after for loop declaration.", it->get_line_number(), it->get_char_index(), 0);
            ++it;
            statement::ptr for_looped = parse_statement(context, it, nullptr, true, allow_return, true, true, false, true, false);
            parsed_stmt = std::make_shared<statement_for>(parent, for_first, for_condition, for_on_iter, for_looped, line_number, char_index);
            for_first->set_parent(parsed_stmt);
            for_condition->set_parent(parsed_stmt);
            for_on_iter->set_parent(parsed_stmt);
            for_looped->set_parent(parsed_stmt);
          }
          break;
        }

        // Statement declares an identifier
        case reserved_token::kw_final:
          parsed_stmt = parse_declare(context, ++it, parent, nullptr, true, false, line_number, char_index);
          break;
        case reserved_token::kw_static:
          parsed_stmt = parse_declare(context, ++it, parent, nullptr, false, true, line_number, char_index);
          break;
        case reserved_token::typekw_bool:
          parsed_stmt = parse_declare(context, ++it, parent, type_registry::bool_handle(), false, false, line_number, char_index);
          break;
        case reserved_token::typekw_byte:
          parsed_stmt = parse_declare(context, ++it, parent, type_registry::byte_handle(), false, false, line_number, char_index);
          break;
        case reserved_token::typekw_char:
          parsed_stmt = parse_declare(context, ++it, parent, type_registry::char_handle(), false, false, line_number, char_index);
          break;
        case reserved_token::typekw_double:
          parsed_stmt = parse_declare(context, ++it, parent, type_registry::double_handle(), false, false, line_number, char_index);
          break;
        case reserved_token::typekw_float:
          parsed_stmt = parse_declare(context, ++it, parent, type_registry::float_handle(), false, false, line_number, char_index);
          break;
        case reserved_token::typekw_int:
          parsed_stmt = parse_declare(context, ++it, parent, type_registry::int_handle(), false, false, line_number, char_index);
          break;
        case reserved_token::typekw_long:
          parsed_stmt = parse_declare(context, ++it, parent, type_registry::long_handle(), false, false, line_number, char_index);
          break;
        case reserved_token::typekw_short:
          parsed_stmt = parse_declare(context, ++it, parent, type_registry::short_handle(), false, false, line_number, char_index);
          break;
        case reserved_token::typekw_str:
          parsed_stmt = parse_declare(context, ++it, parent, type_registry::str_handle(), false, false, line_number, char_index);
          break;
        case reserved_token::typekw_void:
          parsed_stmt = parse_declare(context, ++it, parent, type_registry::void_handle(), false, false, line_number, char_index);
          break;

        // Statement contains only an expression
        default:
          is_expr = true;
          break;
      }
    } else {
      // Statement contains only an expression
      is_expr = true;
    }

    if (is_expr) {
      node_ptr expr_root = parse_expression_tree(context, it, type_registry::void_handle(), false, false, false);
      parsed_stmt = std::make_shared<statement_expr>(parent, expr_root, line_number, char_index);
    }

    // Failsafe to prevent potential issues with assuming that parsed_stmt is present
    if (!parsed_stmt || (!allow_empty && parsed_stmt->get_stmt_type() == statement_type::empty_s))
      throw syntax_error("expected a statement at this position.", line_number, char_index, 0);

    return parsed_stmt;
  }

  std::vector<statement::ptr>& parse_code_statements(compiler_context& context, tokens_iterator& it, std::vector<statement::ptr>& parsed_code) {
    statement::ptr code_parent = std::make_shared<statement_block>(nullptr, parsed_code, 0, 0);
    while (!it->is_eof()) {
      // std::cout << "parsing statement" << std::endl;
      statement::ptr parsed_stmt = parse_statement(context, it, code_parent, true, false, false, false, false, true, false);
      if (parsed_stmt && parsed_stmt->get_stmt_type() != statement_type::empty_s) {
        parsed_code.push_back(parsed_stmt);
      }
      ++it;
    }
    // std::cout << "done parsing" << std::endl;
    return parsed_code;
  }
}
