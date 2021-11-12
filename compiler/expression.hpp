#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <memory>
#include <variant>
#include <vector>

#include "compiler_context.hpp"
#include "runtime_context.hpp"
#include "tokens.hpp"
#include "types.hpp"
#include "variable.hpp"

namespace valley {
  enum struct node_operation {
    un_preinc,
    un_postinc,
    un_predec,
    un_postdec,
    
    un_pos,
    un_neg,
    un_bwnot,

    un_lnot,

    bin_add,
    bin_sub,
    bin_mul,
    bin_div,
    bin_mod,
    bin_pow,
    bin_bwand,
    bin_bwor,
    bin_bwxor,
    bin_lshift,
    bin_rshift,
    
    bin_asg,
    bin_asg_add,
    bin_asg_sub,
    bin_asg_mul,
    bin_asg_div,
    bin_asg_mod,
    bin_asg_pow,
    bin_asg_and,
    bin_asg_or,
    bin_asg_xor,
    bin_asg_lshift,
    bin_asg_rshift,

    bin_eq,
    bin_neq,
    bin_lt,
    bin_gt,
    bin_lteq,
    bin_gteq,

    bin_land,
    bin_lor,
    bin_lxor,

    ternary,
    comma,
    index,
    call,
    list,
  };

  struct declaration {
    std::string name;
    type_handle type_id;
    bool is_final;
    bool is_static;
  }

  struct node;
  using node_ptr = std::unique_ptr<node>;
  using node_value = std::variant<node_operation, void_value, int8_t, int16_t, int32_t, int64_t, float, double, bool, char, std::string, identifier, declaration>;
  
  struct node {
    private:
      node_value _value;
      std::vector<node_ptr> _children;
      type_handle _type_id;
      bool _lvalue;
      size_t _line_number;
      size_t _char_index;

    public:
      node(compiler_context& context, node_value value, std::vector<node_ptr> children, size_t line_number, size_t char_index);

      bool is_operation() const;
      bool is_identifier() const;
      bool is_declaration() const;
      bool is_null() const;
      bool is_byte() const;
      bool is_short() const;
      bool is_int() const;
      bool is_long() const;
      bool is_float() const;
      bool is_double() const;
      bool is_bool() const;
      bool is_char() const;
      bool is_str() const;

      const node_value& get_value() const;

      node_operation get_operation() const;
      std::string_view get_identifier() const;
      declaration get_declaration() const;
      int8_t get_byte() const;
      int16_t get_short() const;
      int32_t get_int() const;
      int64_t get_long() const;
      float get_float() const;
      double get_double() const;
      bool get_bool() const;
      char get_char() const;
      std::string_view get_str() const;
      
      const std::vector<node_ptr>& get_children() const;
      type_handle get_type_id() const;
      bool is_lvalue() const;
      size_t get_line_number() const;
      size_t get_char_index() const;

      void check_conversion(type_handle type_id, bool lvalue) const;

      const var_value& evaluate(runtime_context& context) const;
  };
}

namespace std {
  string to_string(const valley::node_ptr& n);
}

#endif
