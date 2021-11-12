#include "expression_manager.hpp"

#include <stack>
#include <iostream>

#include "errors.hpp"
#include "tokenizer.hpp"

namespace valley {
  namespace {
    enum struct operator_precedence {
      postfix,
      prefix,
      exponentative,
      multiplicative,
      additive,
      shift,
      inequality,
      equality,
      bitwise_and,
      bitwise_xor,
      bitwise_or,
      logical_and,
      logical_xor,
      logical_or,
      assignment,
      comma,
      container,
    };

    enum struct operator_associativity {
      left_to_right,
      right_to_left,
    };

    struct operator_info {
      node_operation operation;
      operator_precedence precedence;
      operator_associativity associativity;
      int num_operands;
      size_t line_number;
      size_t char_index;

      operator_info(node_operation operation, size_t line_number, size_t char_index):
      operation(operation), line_number(line_number), char_index(char_index) {
        switch (operation) {
          case node_operation::un_postinc:
          case node_operation::un_postdec:
          case node_operation::index:
          case node_operation::call:
            precedence = operator_precedence::postfix;
            break;
          case node_operation::un_preinc:
          case node_operation::un_predec:
          case node_operation::un_pos:
          case node_operation::un_neg:
          case node_operation::un_bwnot:
          case node_operation::un_lnot:
            precedence = operator_precedence::prefix;
            break;
          case node_operation::bin_pow:
            precedence = operator_precedence::exponentative;
            break;
          case node_operation::bin_mul:
          case node_operation::bin_div:
          case node_operation::bin_mod:
            precedence = operator_precedence::multiplicative;
            break;
          case node_operation::bin_add:
          case node_operation::bin_sub:
            precedence = operator_precedence::additive;
            break;
          case node_operation::bin_lshift:
          case node_operation::bin_rshift:
            precedence = operator_precedence::shift;
            break;
          case node_operation::bin_lt:
          case node_operation::bin_gt:
          case node_operation::bin_lteq:
          case node_operation::bin_gteq:
            precedence = operator_precedence::inequality;
            break;
          case node_operation::bin_eq:
          case node_operation::bin_neq:
            precedence = operator_precedence::equality;
            break;
          case node_operation::bin_bwand:
            precedence = operator_precedence::bitwise_and;
            break;
          case node_operation::bin_bwxor:
            precedence = operator_precedence::bitwise_xor;
            break;
          case node_operation::bin_bwor:
            precedence = operator_precedence::bitwise_or;
            break;
          case node_operation::bin_land:
            precedence = operator_precedence::logical_and;
            break;
          case node_operation::bin_lxor:
            precedence = operator_precedence::logical_xor;
            break;
          case node_operation::bin_lor:
            precedence = operator_precedence::logical_or;
            break;
          case node_operation::bin_asg:
          case node_operation::bin_asg_add:
          case node_operation::bin_asg_sub:
          case node_operation::bin_asg_mul:
          case node_operation::bin_asg_div:
          case node_operation::bin_asg_mod:
          case node_operation::bin_asg_pow:
          case node_operation::bin_asg_and:
          case node_operation::bin_asg_or:
          case node_operation::bin_asg_xor:
          case node_operation::bin_asg_lshift:
          case node_operation::bin_asg_rshift:
          case node_operation::ternary:
            precedence = operator_precedence::assignment;
            break;
          case node_operation::comma:
            precedence = operator_precedence::comma;
            break;     
          case node_operation::list:
            precedence = operator_precedence::container;
            break;     
        }

        switch (precedence) {
          case operator_precedence::assignment:
          case operator_precedence::prefix:
            associativity = operator_associativity::right_to_left;
            break;
          default:
            associativity = operator_associativity::left_to_right;
            break;
        }

        switch (operation) {
          case node_operation::un_preinc:
          case node_operation::un_predec:
          case node_operation::un_postinc:
          case node_operation::un_postdec:
          case node_operation::un_pos:
          case node_operation::un_neg:
          case node_operation::un_bwnot:
          case node_operation::un_lnot:
          case node_operation::comma:
          case node_operation::call:
          case node_operation::list:
            num_operands = 1;
            break;
          case node_operation::ternary:
            num_operands = 3;
            break;
          default:
            num_operands = 2;
            break;
        }
      }
    };

    operator_info get_operator_info(reserved_token rtoken, bool prefix, size_t line_number, size_t char_index) {
      switch (rtoken) {
        case reserved_token::d_plus:
          return prefix
            ? operator_info(node_operation::un_preinc, line_number, char_index)
            : operator_info(node_operation::un_postinc, line_number, char_index);
        case reserved_token::d_minus:
          return prefix
            ? operator_info(node_operation::un_predec, line_number, char_index)
            : operator_info(node_operation::un_postdec, line_number, char_index);
        case reserved_token::plus:
          return prefix
            ? operator_info(node_operation::un_pos, line_number, char_index)
            : operator_info(node_operation::bin_add, line_number, char_index);
        case reserved_token::minus:
          return prefix
            ? operator_info(node_operation::un_neg, line_number, char_index)
            : operator_info(node_operation::bin_sub, line_number, char_index);
        case reserved_token::star:
          return operator_info(node_operation::bin_mul, line_number, char_index);
        case reserved_token::slash:
          return operator_info(node_operation::bin_div, line_number, char_index);
        case reserved_token::prcnt:
          return operator_info(node_operation::bin_mod, line_number, char_index);
        case reserved_token::d_star:
          return operator_info(node_operation::bin_pow, line_number, char_index);
        case reserved_token::tilde:
          return operator_info(node_operation::un_bwnot, line_number, char_index);
        case reserved_token::ampr:
          return operator_info(node_operation::bin_bwand, line_number, char_index);
        case reserved_token::vbar:
          return operator_info(node_operation::bin_bwor, line_number, char_index);
        case reserved_token::caret:
          return operator_info(node_operation::bin_bwxor, line_number, char_index);
        case reserved_token::d_langle:
          return operator_info(node_operation::bin_lshift, line_number, char_index);
        case reserved_token::d_rangle:
          return operator_info(node_operation::bin_rshift, line_number, char_index);
        case reserved_token::equal:
          return operator_info(node_operation::bin_asg, line_number, char_index);
        case reserved_token::plus_eq:
          return operator_info(node_operation::bin_asg_add, line_number, char_index);
        case reserved_token::minus_eq:
          return operator_info(node_operation::bin_asg_sub, line_number, char_index);
        case reserved_token::star_eq:
          return operator_info(node_operation::bin_asg_mul, line_number, char_index);
        case reserved_token::slash_eq:
          return operator_info(node_operation::bin_asg_div, line_number, char_index);
        case reserved_token::prcnt_eq:
          return operator_info(node_operation::bin_asg_mod, line_number, char_index);
        case reserved_token::d_star_eq:
          return operator_info(node_operation::bin_asg_pow, line_number, char_index);
        case reserved_token::ampr_eq:
          return operator_info(node_operation::bin_asg_and, line_number, char_index);
        case reserved_token::vbar_eq:
          return operator_info(node_operation::bin_asg_or, line_number, char_index);
        case reserved_token::caret_eq:
          return operator_info(node_operation::bin_asg_xor, line_number, char_index);
        case reserved_token::d_langle_eq:
          return operator_info(node_operation::bin_asg_lshift, line_number, char_index);
        case reserved_token::d_rangle_eq:
          return operator_info(node_operation::bin_asg_rshift, line_number, char_index);
        case reserved_token::exclm:
          return operator_info(node_operation::un_lnot, line_number, char_index);
        case reserved_token::d_ampr:
          return operator_info(node_operation::bin_land, line_number, char_index);
        case reserved_token::d_vbar:
          return operator_info(node_operation::bin_lor, line_number, char_index);
        case reserved_token::d_caret:
          return operator_info(node_operation::bin_lxor, line_number, char_index);
        case reserved_token::d_equal:
          return operator_info(node_operation::bin_eq, line_number, char_index);
        case reserved_token::exclm_eq:
          return operator_info(node_operation::bin_neq, line_number, char_index);
        case reserved_token::langle:
          return operator_info(node_operation::bin_lt, line_number, char_index);
        case reserved_token::rangle:
          return operator_info(node_operation::bin_gt, line_number, char_index);
        case reserved_token::langle_eq:
          return operator_info(node_operation::bin_lteq, line_number, char_index);
        case reserved_token::rangle_eq:
          return operator_info(node_operation::bin_gteq, line_number, char_index);
        case reserved_token::qmark:
          return operator_info(node_operation::ternary, line_number, char_index);
        case reserved_token::comma:
          return operator_info(node_operation::comma, line_number, char_index);
        case reserved_token::lparen:
          return operator_info(node_operation::call, line_number, char_index);
        case reserved_token::lbrckt:
          return operator_info(node_operation::index, line_number, char_index);
        default:
          throw unexpected_syntax_error(std::to_string(rtoken), line_number, char_index, false);
      }
    }

    bool is_end_of_expression(const token& t, bool allow_comma) {
      if (t.is_reserved_token()) {
        switch (t.get_reserved_token()) {
          case reserved_token::semic:
          case reserved_token::colon:
          case reserved_token::rparen:
          case reserved_token::rbrckt:
          case reserved_token::rbrace:
            return true;
          case reserved_token::comma:
            return !allow_comma;
          default:
            return false;
        }
      }
      return t.is_eof();
    }

    bool is_evaluated_before(const operator_info& l, const operator_info& r) {
      return l.associativity == operator_associativity::left_to_right ? l.precedence <= r.precedence : l.precedence < r.precedence;
    }

    void pop_one_operator(std::stack<operator_info>& operator_stack, std::stack<node_ptr>& operand_stack, compiler_context& context, size_t line_number, size_t char_index) {
      if (operand_stack.size() < operator_stack.top().num_operands) {
        std::string str = "failed to parse expression. (expected ";
        str += std::to_string(operator_stack.top().num_operands);
        str += " operands, got ";
        str += std::to_string(operand_stack.size());
        throw compiler_error(str + ".)", line_number, char_index, 0);
      }

      std::vector<node_ptr> operands;
      operands.resize(operator_stack.top().num_operands);

      if (operator_stack.top().precedence != operator_precedence::prefix) {
        operator_stack.top().line_number = operand_stack.top()->get_line_number();
        operator_stack.top().char_index = operand_stack.top()->get_char_index();
      }

      for (int i = operator_stack.top().num_operands - 1; i >= 0; --i) {
        operands[i] = std::move(operand_stack.top());
        operand_stack.pop();
      }

      operand_stack.push(std::make_unique<node>(context, operator_stack.top().operation, std::move(operands), operator_stack.top().line_number, operator_stack.top().char_index));

      operator_stack.pop();
    }

    node_ptr parse_expression_tree_impl(compiler_context& context, tokens_iterator& it, bool allow_comma, bool allow_empty) {
      std::stack<node_ptr> operand_stack;
      std::stack<operator_info> operator_stack;

      bool expecting_operand = true;
      for (; !is_end_of_expression(*it, allow_comma); ++it) {
        if (it->is_reserved_token()) {
          operator_info oi = get_operator_info(it->get_reserved_token(), expecting_operand, it->get_line_number(), it->get_char_index());

          if (oi.operation == node_operation::call && expecting_operand) {
            // Expression grouping, (expr)
            ++it;
            operand_stack.push(parse_expression_tree_impl(context, it, false, false));
            if (!it->has_value(reserved_token::rparen))
              throw syntax_error("could not find a matching ')'.", oi.line_number, oi.char_index, 0);
            expecting_operand = false;
            continue;
          }

          if (oi.operation == node_operation::index && expecting_operand) {
            // Array literal, [item, ...]
            oi.operation = node_operation::list;
            oi.precedence = operator_precedence::container;
            oi.num_operands = 0;
            ++it;
            if (it->has_value(reserved_token::rbrckt))
              break;
            while (true) {
              node_ptr item = parse_expression_tree_impl(context, it, false, false);
              operand_stack.push(std::move(item));
              ++oi.num_operands;
              if (it->has_value(reserved_token::rbrckt))
                break;
              if (!it->has_value(reserved_token::comma))
                throw syntax_error("could not find a matching ']'.", oi.line_number, oi.char_index, 0);
              ++it;
            }
            operator_stack.push(oi);
            expecting_operand = false;
            continue;
          }

          if ((oi.precedence == operator_precedence::prefix) != expecting_operand) {
            throw unexpected_syntax_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), true);
          }

          if (!operator_stack.empty() && is_evaluated_before(operator_stack.top(), oi)) {
            pop_one_operator(operator_stack, operand_stack, context, it->get_line_number(), it->get_char_index());
          }

          switch (oi.operation) {
            case node_operation::call: {
              ++it;
              if (it->has_value(reserved_token::rparen))
                break;
              while (true) {
                node_ptr argument = parse_expression_tree_impl(context, it, false, false);
                operand_stack.push(std::move(argument));
                ++oi.num_operands;
                if (it->has_value(reserved_token::rparen))
                  break;
                if (!it->has_value(reserved_token::comma))
                  throw syntax_error("could not find a matching ')'.", it->get_line_number(), it->get_char_index(), 0);
                ++it;
              }
              break;
            }
            case node_operation::index:
              ++it;
              operand_stack.push(parse_expression_tree_impl(context, it, true, false));
              if (!it->has_value(reserved_token::rbrckt))
                throw syntax_error("could not find a matching ']'.", it->get_line_number(), it->get_char_index(), 0);
              break;
            case node_operation::ternary:
              ++it;
              operand_stack.push(parse_expression_tree_impl(context, it, false, false));
              if (!it->has_value(reserved_token::colon))
                throw syntax_error("expected ':' in ternary expression.", it->get_line_number(), it->get_char_index(), 0);
              break;
            default:
              break;
          }

          operator_stack.push(oi);
          expecting_operand = oi.precedence != operator_precedence::postfix;
        } else {
          if (!expecting_operand) {
            throw unexpected_syntax_error(std::to_string(*it), it->get_line_number(), it->get_char_index(), false);
          }
          node_value value;
          if (it->is_null()) {
            node_value(void_value{});
          } else if (it->is_byte()) {
            node_value(it->get_byte());
          } else if (it->is_short()) {
            node_value(it->get_short());
          } else if (it->is_int()) {
            node_value(it->get_int());
          } else if (it->is_long()) {
            node_value(it->get_long());
          } else if (it->is_float()) {
            node_value(it->get_float());
          } else if (it->is_double()) {
            node_value(it->get_double());
          } else if (it->is_bool()) {
            node_value(it->get_bool());
          } else if (it->is_char()) {
            node_value(it->get_char());
          } else if (it->is_str()) {
            node_value(it->get_str());
          } else if (it->is_identifier()) {
            node_value(it->get_identifier());
          }
          operand_stack.push(std::make_unique<node>(context, value, std::vector<node_ptr>(), it->get_line_number(), it->get_char_index()));
          expecting_operand = false;
        }
      }

      if (expecting_operand) {
        if (allow_empty && operand_stack.empty() && operator_stack.empty()) {
          return node_ptr();
        } else {
          throw syntax_error("expected an operand.", it->get_line_number(), it->get_char_index(), 0);
        }
      }

      while (!operator_stack.empty()) {
        pop_one_operator(operator_stack, operand_stack, context, it->get_line_number(), it->get_char_index());
      }

      if (operand_stack.size() != 1 || !operator_stack.empty()) {
        std::string str = "failed to parse expression. (resolved to ";
        str += std::to_string(operand_stack.size());
        str += " operands, ";
        str += std::to_string(operator_stack.size());
        throw compiler_error(str + " operators.)", it->get_line_number(), it->get_char_index(), 0);
      }

      return std::move(operand_stack.top());
    }
  }

  node_ptr parse_expression_tree(compiler_context& context, tokens_iterator& it, type_handle type_id, bool lvalue, bool allow_comma, bool allow_empty) {
    node_ptr ret = parse_expression_tree_impl(context, it, allow_comma, allow_empty);
    ret->check_conversion(type_id, lvalue);
    return ret;
  }
}