#include "expression.hpp"

#include <cstdint>
#include <iostream>

#include "errors.hpp"
#include "helpers.hpp"

namespace valley {
  namespace {
    bool is_convertible(type_handle type_from, bool lvalue_from, type_handle type_to, bool lvalue_to) {
      if ((type_from == type_to && lvalue_from == lvalue_to) || type_to == type_registry::void_handle()) {
        return true;
      }
      if (lvalue_to) {
        return lvalue_from && is_convertible(type_from, false, type_to, false);
      }
      if (std::holds_alternative<any_type>(*type_to) || type_to == type_registry::bool_handle()) {
        return true;
      }
      if (std::holds_alternative<array_type>(*type_from) && std::holds_alternative<array_type>(*type_to)) {
        return is_convertible(std::get<array_type>(*type_from).inner, lvalue_from, std::get<array_type>(*type_to).inner, lvalue_to);
      }
      if (type_from == type_registry::bool_handle()) {
        return type_to == type_registry::byte_handle()
          || type_to == type_registry::short_handle()
          || type_to == type_registry::int_handle()
          || type_to == type_registry::long_handle()
          || type_to == type_registry::float_handle()
          || type_to == type_registry::double_handle()
          || type_to == type_registry::char_handle();
      }
      if (type_from == type_registry::byte_handle() || type_from == type_registry::char_handle()) { // fix. :(
        return type_to == type_registry::byte_handle()
          || type_to == type_registry::short_handle()
          || type_to == type_registry::int_handle()
          || type_to == type_registry::long_handle()
          || type_to == type_registry::float_handle()
          || type_to == type_registry::double_handle()
          || type_to == type_registry::char_handle();
      }
      if (type_from == type_registry::short_handle()) {
        return type_to == type_registry::short_handle()
          || type_to == type_registry::int_handle()
          || type_to == type_registry::long_handle()
          || type_to == type_registry::float_handle()
          || type_to == type_registry::double_handle();
      }
      if (type_from == type_registry::int_handle()) {
        return type_to == type_registry::int_handle()
          || type_to == type_registry::long_handle()
          || type_to == type_registry::float_handle()
          || type_to == type_registry::double_handle();
      }
      if (type_from == type_registry::long_handle()) {
        return type_to == type_registry::long_handle()
          || type_to == type_registry::float_handle()
          || type_to == type_registry::double_handle();
      }
      if (type_from == type_registry::float_handle()) {
        return type_to == type_registry::float_handle()
          || type_to == type_registry::double_handle();
      }
      if (type_from == type_registry::double_handle()) {
        return type_to == type_registry::double_handle();
      }
      return type_to == type_registry::str_handle();
    }

    bool is_numeric(type_handle t) {
      return t == type_registry::double_handle()
        || t == type_registry::float_handle()
        || t == type_registry::long_handle()
        || t == type_registry::int_handle()
        || t == type_registry::short_handle()
        || t == type_registry::byte_handle()
        || t == type_registry::char_handle()
        || t == type_registry::bool_handle();
    }

    type_handle max_numeric_precision(type_handle t1, type_handle t2) {
      if (!is_numeric(t1) || !is_numeric(t2)) {
        return type_registry::void_handle();
      }
      if (t1 == type_registry::double_handle() || t2 == type_registry::double_handle()) {
        return type_registry::double_handle();
      }
      if (t1 == type_registry::float_handle() || t2 == type_registry::float_handle()) {
        return type_registry::float_handle();
      }
      if (t1 == type_registry::long_handle() || t2 == type_registry::long_handle()) {
        return type_registry::long_handle();
      }
      if (t1 == type_registry::int_handle() || t2 == type_registry::int_handle()) {
        return type_registry::int_handle();
      }
      if (t1 == type_registry::short_handle() || t2 == type_registry::short_handle()) {
        return type_registry::short_handle();
      }
      if (t1 == type_registry::byte_handle() || t2 == type_registry::byte_handle()) {
        return type_registry::byte_handle();
      }
      if (t1 == type_registry::char_handle() || t2 == type_registry::char_handle()) {
        return type_registry::char_handle();
      }
      if (t1 == type_registry::bool_handle() || t2 == type_registry::bool_handle()) {
        return type_registry::bool_handle();
      }
      return type_registry::void_handle();
    }
  }

  node::node(compiler_context& context, node_value value, std::vector<node_ptr> children, size_t line_number, size_t char_index):
  _value(std::move(value)), _children(std::move(children)), _line_number(line_number), _char_index(char_index) {
    const type_handle any_handle = type_registry::any_handle();
    const type_handle void_handle = type_registry::void_handle();
    const type_handle byte_handle = type_registry::byte_handle();
    const type_handle short_handle = type_registry::short_handle();
    const type_handle int_handle = type_registry::int_handle();
    const type_handle long_handle = type_registry::long_handle();
    const type_handle float_handle = type_registry::float_handle();
    const type_handle double_handle = type_registry::double_handle();
    const type_handle bool_handle = type_registry::bool_handle();
    const type_handle char_handle = type_registry::char_handle();
    const type_handle str_handle = type_registry::str_handle();

    std::visit(overloaded{
      [&](void_value) {
        _type_id = void_handle;
        _lvalue = false;
      },
      [&](int8_t value) {
        _type_id = byte_handle;
        _lvalue = false;
      },
      [&](int16_t value) {
        _type_id = short_handle;
        _lvalue = false;
      },
      [&](int32_t value) {
        _type_id = int_handle;
        _lvalue = false;
      },
      [&](int64_t value) {
        _type_id = long_handle;
        _lvalue = false;
      },
      [&](float value) {
        _type_id = float_handle;
        _lvalue = false;
      },
      [&](double value) {
        _type_id = double_handle;
        _lvalue = false;
      },
      [&](bool value) {
        _type_id = bool_handle;
        _lvalue = false;
      },
      [&](char value) {
        _type_id = char_handle;
        _lvalue = false;
      },
      [&](const std::string& value) {
        _type_id = str_handle;
        _lvalue = false;
      },
      [&](const identifier& value) {
        if (const identifier_info* info = context.find(value.name)) {
          _type_id = info->type_id();
          _lvalue = !info->is_final();
        } else {
          throw undeclared_error(value.name, _line_number, _char_index, 0);
        }
      },
      [&](const declaration& value) {
        if (const identifier_info* info = context.find(value.name)) {
          throw semantic_error("Identifier '" + value.name + "' may already be declared in this scope.", _line_number, _char_index, 0);
        } else {
          _type_id = value.type_id;
          _lvalue = !value.is_final;
        }
      },
      [&](node_operation value) {
        switch (value) {
          case node_operation::un_preinc:
          case node_operation::un_predec:
          case node_operation::un_postinc:
          case node_operation::un_postdec:
            _children[0]->check_conversion(double_handle, true);
            _type_id = _children[0]->_type_id;
            _lvalue = true;
            break;
          case node_operation::un_pos:
          case node_operation::un_neg:
          case node_operation::un_bwnot:
            _children[0]->check_conversion(long_handle, false);
            _type_id = _children[0]->_type_id;
            _lvalue = false;
            break;
          case node_operation::un_lnot:
            _children[0]->check_conversion(bool_handle, false);
            _type_id = bool_handle;
            _lvalue = false;
            break;
          case node_operation::bin_add:
          case node_operation::bin_sub:
          case node_operation::bin_mul:
          case node_operation::bin_div:
          case node_operation::bin_mod:
          case node_operation::bin_pow:
            _children[0]->check_conversion(double_handle, false);
            _children[1]->check_conversion(double_handle, false);
            _type_id = max_numeric_precision(_children[0]->_type_id, _children[1]->_type_id);
            _lvalue = false;
            break;
          case node_operation::bin_bwand:
          case node_operation::bin_bwor:
          case node_operation::bin_bwxor:
          case node_operation::bin_lshift:
          case node_operation::bin_rshift:
            _children[0]->check_conversion(long_handle, false);
            _children[1]->check_conversion(long_handle, false);
            _type_id = max_numeric_precision(_children[0]->_type_id, _children[1]->_type_id);
            _lvalue = false;
            break;
          case node_operation::bin_land:
          case node_operation::bin_lor:
          case node_operation::bin_lxor:
            _children[0]->check_conversion(bool_handle, false);
            _children[1]->check_conversion(bool_handle, false);
            _type_id = bool_handle;
            _lvalue = false;
            break;
          case node_operation::bin_lt:
          case node_operation::bin_gt:
          case node_operation::bin_lteq:
          case node_operation::bin_gteq:
            _children[0]->check_conversion(double_handle, false);
            _children[1]->check_conversion(double_handle, false);
          case node_operation::bin_eq:
          case node_operation::bin_neq:
            _type_id = bool_handle;
            _lvalue = false;
            break;
          case node_operation::bin_asg:
            _type_id = _children[0]->_type_id;
            _lvalue = true;
            _children[0]->check_conversion(_type_id, true);
            _children[1]->check_conversion(_type_id, false);
            break;
          case node_operation::bin_asg_add:
          case node_operation::bin_asg_sub:
          case node_operation::bin_asg_mul:
          case node_operation::bin_asg_div:
          case node_operation::bin_asg_mod:
          case node_operation::bin_asg_pow:
            _children[0]->check_conversion(double_handle, true);
            _children[1]->check_conversion(double_handle, false);
            _type_id = _children[0]->_type_id;
            _lvalue = true;
            break;
          case node_operation::bin_asg_and:
          case node_operation::bin_asg_or:
          case node_operation::bin_asg_xor:
          case node_operation::bin_asg_lshift:
          case node_operation::bin_asg_rshift:
            _children[0]->check_conversion(long_handle, true);
            _children[1]->check_conversion(long_handle, false);
            _type_id = _children[0]->_type_id;
            _lvalue = true;
            break;
          case node_operation::comma:
            _type_id = _children.back()->get_type_id();
            _lvalue = _children.back()->is_lvalue();
            break;
          case node_operation::index:
            if (const array_type* at = std::get_if<array_type>(_children[0]->get_type_id())) {
              _type_id = at->inner;
              _lvalue = _children[0]->is_lvalue();
            } else if (_children[0]->get_type_id() == str_handle) {
              _type_id = char_handle;
              _lvalue = false;
            } else {
              throw semantic_error(to_string(_children[0]->get_type_id()) + " is not indexable.", _line_number, _char_index, 0);
            }
            break;
          case node_operation::ternary:
            _children[0]->check_conversion(bool_handle, false);
            if (is_convertible(
              _children[2]->_type_id, _children[2]->_lvalue,
              _children[1]->_type_id, _children[1]->_lvalue
            )) {
              _children[2]->check_conversion(_children[1]->_type_id, _children[1]->_lvalue);
              _type_id = _children[1]->_type_id;
              _lvalue = _children[1]->_lvalue;
            } else {
              _children[1]->check_conversion(_children[2]->_type_id, _children[2]->_lvalue);
              _type_id = _children[2]->_type_id;
              _lvalue = _children[2]->_lvalue;
            }
            break;
          case node_operation::call:
            if (const function_type* ft = std::get_if<function_type>(_children[0]->_type_id)) {
              _type_id = ft->returning;
              _lvalue = false;
              if (ft->has_varargs) {
                if (const array_type* varargs = std::get_if<array_type>(ft->params.back()))
                for (size_t i = 0; i < _children.size() - 1; ++i) {
                  if (i >= ft->params.size() - 1)
                    _children[i + 1]->check_conversion(varargs->inner, false);
                  else
                    _children[i + 1]->check_conversion(ft->params[i], false);
                } else
                  throw compiler_error("varargs not working properly in function signature.", _line_number, _char_index, 1);
              } else {
                if (ft->params.size() != _children.size() - 1) {
                  throw semantic_error("expected " + std::to_string(ft->params.size()) + " arguments, got " + std::to_string(_children.size() - 1) + " instead.", _line_number, _char_index, 0);
                }
                for (size_t i = 0; i < ft->params.size(); ++i) {
                  _children[i + 1]->check_conversion(ft->params[i], false);
                }
              }
            } else
              throw semantic_error("'" + std::to_string(_children[0]->_type_id) + "' object is not callable.", _line_number, _char_index, 0);
            break;
          case node_operation::list:
            _type_id = _children.back()->get_type_id(); // Type to check against (temporary)
            for (int i = 0; i < int(_children.size()) - 1; ++i) {
              _children[i]->check_conversion(_type_id, false);
            }
            _type_id = context.get_handle(array_type{_type_id});
            _lvalue = false;
        }
      }
    }, _value);
  }

  bool node::is_operation() const {
    return std::holds_alternative<node_operation>(_value);
  }

  bool node::is_identifier() const {
    return std::holds_alternative<identifier>(_value);
  }

  bool node::is_declaration() const {
    return std::holds_alternative<declaration>(_value);
  }

  bool node::is_null() const {
    return std::holds_alternative<void_value>(_value);
  }

  bool node::is_byte() const {
    return std::holds_alternative<int8_t>(_value);
  }

  bool node::is_short() const {
    return std::holds_alternative<int16_t>(_value);
  }

  bool node::is_int() const {
    return std::holds_alternative<int32_t>(_value);
  }

  bool node::is_long() const {
    return std::holds_alternative<int64_t>(_value);
  }

  bool node::is_float() const {
    return std::holds_alternative<float>(_value);
  }

  bool node::is_double() const {
    return std::holds_alternative<double>(_value);
  }

  bool node::is_bool() const {
    return std::holds_alternative<bool>(_value);
  }

  bool node::is_char() const {
    return std::holds_alternative<char>(_value);
  }

  bool node::is_str() const {
    return std::holds_alternative<std::string>(_value);
  }

  const node_value& node::get_value() const {
    return _value;
  }

  node_operation node::get_operation() const {
    return std::get<node_operation>(_value);
  }

  std::string_view node::get_identifier() const {
    return std::get<identifier>(_value).name;
  }

  declaration node::get_declaration() const {
    return std::get<declaration>(_value);
  }

  int8_t node::get_byte() const {
    return std::get<int8_t>(_value);
  }

  int16_t node::get_short() const {
    return std::get<int16_t>(_value);
  }

  int32_t node::get_int() const {
    return std::get<int32_t>(_value);
  }

  int64_t node::get_long() const {
    return std::get<int64_t>(_value);
  }

  float node::get_float() const {
    return std::get<float>(_value);
  }

  double node::get_double() const {
    return std::get<double>(_value);
  }

  bool node::get_bool() const {
    return std::get<bool>(_value);
  }

  char node::get_char() const {
    return std::get<char>(_value);
  }

  std::string_view node::get_str() const {
    return std::get<std::string>(_value);
  }

  const std::vector<node_ptr>& node::get_children() const {
    return _children;
  }

  type_handle node::get_type_id() const {
    return _type_id;
  }

  bool node::is_lvalue() const {
    return _lvalue;
  }

  size_t node::get_line_number() const {
    return _line_number;
  }

  size_t node::get_char_index() const {
    return _char_index;
  }

  void node::check_conversion(type_handle type_id, bool lvalue) const {
    if (!is_convertible(_type_id, _lvalue, type_id, lvalue)) {
      if (!is_convertible(_type_id, _lvalue, type_id, false))
        throw wrong_type_error(std::to_string(_type_id), std::to_string(type_id), _line_number, _char_index, 0);
      throw not_lvalue_error(std::to_string(_type_id), _line_number, _char_index, 0);
    }
  }

  /*const var_value& node::evaluate(runtime_context& context) const {
    const type_handle any_handle = type_registry::any_handle();
    const type_handle void_handle = type_registry::void_handle();
    const type_handle byte_handle = type_registry::byte_handle();
    const type_handle short_handle = type_registry::short_handle();
    const type_handle int_handle = type_registry::int_handle();
    const type_handle long_handle = type_registry::long_handle();
    const type_handle float_handle = type_registry::float_handle();
    const type_handle double_handle = type_registry::double_handle();
    const type_handle bool_handle = type_registry::bool_handle();
    const type_handle char_handle = type_registry::char_handle();
    const type_handle str_handle = type_registry::str_handle();
    
    if (is_operation()) {
      var_value val;
      switch (get_operation()) {
        case node_operation::un_preinc:
          val = _children[0]->evaluate(context);
          if (!std::holds_alternative<lvar_value>(val))
            throw runtime_error("expected an assignable value.", _line_number, _char_index);
          ++std::get<lvar_value>(val)->value;
        case node_operation::un_predec:
        case node_operation::un_postinc:
        case node_operation::un_postdec:
          _children[0]->check_conversion(double_handle, true);
          _type_id = _children[0]->_type_id;
          _lvalue = true;
          break;
        case node_operation::un_pos:
        case node_operation::un_neg:
        case node_operation::un_bwnot:
          _children[0]->check_conversion(long_handle, false);
          _type_id = _children[0]->_type_id;
          _lvalue = false;
          break;
        case node_operation::un_lnot:
          _children[0]->check_conversion(bool_handle, false);
          _type_id = bool_handle;
          _lvalue = false;
          break;
        case node_operation::bin_add:
        case node_operation::bin_sub:
        case node_operation::bin_mul:
        case node_operation::bin_div:
        case node_operation::bin_mod:
        case node_operation::bin_pow:
          _children[0]->check_conversion(double_handle, false);
          _children[1]->check_conversion(double_handle, false);
          _type_id = max_numeric_precision(_children[0]->_type_id, _children[1]->_type_id);
          _lvalue = false;
          break;
        case node_operation::bin_bwand:
        case node_operation::bin_bwor:
        case node_operation::bin_bwxor:
        case node_operation::bin_lshift:
        case node_operation::bin_rshift:
          _children[0]->check_conversion(long_handle, false);
          _children[1]->check_conversion(long_handle, false);
          _type_id = max_numeric_precision(_children[0]->_type_id, _children[1]->_type_id);
          _lvalue = false;
          break;
        case node_operation::bin_land:
        case node_operation::bin_lor:
        case node_operation::bin_lxor:
          _children[0]->check_conversion(bool_handle, false);
          _children[1]->check_conversion(bool_handle, false);
          _type_id = bool_handle;
          _lvalue = false;
          break;
        case node_operation::bin_lt:
        case node_operation::bin_gt:
        case node_operation::bin_lteq:
        case node_operation::bin_gteq:
          _children[0]->check_conversion(double_handle, false);
          _children[1]->check_conversion(double_handle, false);
        case node_operation::bin_eq:
        case node_operation::bin_neq:
          _type_id = bool_handle;
          _lvalue = false;
          break;
        case node_operation::bin_asg:
          _type_id = _children[0]->_type_id;
          _lvalue = true;
          _children[0]->check_conversion(_type_id, true);
          _children[1]->check_conversion(_type_id, false);
          break;
        case node_operation::bin_asg_add:
        case node_operation::bin_asg_sub:
        case node_operation::bin_asg_mul:
        case node_operation::bin_asg_div:
        case node_operation::bin_asg_mod:
        case node_operation::bin_asg_pow:
          _children[0]->check_conversion(double_handle, true);
          _children[1]->check_conversion(double_handle, false);
          _type_id = _children[0]->_type_id;
          _lvalue = true;
          break;
        case node_operation::bin_asg_and:
        case node_operation::bin_asg_or:
        case node_operation::bin_asg_xor:
        case node_operation::bin_asg_lshift:
        case node_operation::bin_asg_rshift:
          _children[0]->check_conversion(long_handle, true);
          _children[1]->check_conversion(long_handle, false);
          _type_id = _children[0]->_type_id;
          _lvalue = true;
          break;
        case node_operation::comma:
          _type_id = _children.back()->get_type_id();
          _lvalue = _children.back()->is_lvalue();
          break;
        case node_operation::index:
          if (const array_type* at = std::get_if<array_type>(_children[0]->get_type_id())) {
            _type_id = at->inner;
            _lvalue = _children[0]->is_lvalue();
          } else if (_children[0]->get_type_id() == str_handle) {
            _type_id = char_handle;
            _lvalue = false;
          } else {
            throw semantic_error(to_string(_children[0]->get_type_id()) + " is not indexable.", _line_number, _char_index, 0);
          }
          break;
        case node_operation::ternary:
          _children[0]->check_conversion(bool_handle, false);
          if (is_convertible(
            _children[2]->_type_id, _children[2]->_lvalue,
            _children[1]->_type_id, _children[1]->_lvalue
          )) {
            _children[2]->check_conversion(_children[1]->_type_id, _children[1]->_lvalue);
            _type_id = _children[1]->_type_id;
            _lvalue = _children[1]->_lvalue;
          } else {
            _children[1]->check_conversion(_children[2]->_type_id, _children[2]->_lvalue);
            _type_id = _children[2]->_type_id;
            _lvalue = _children[2]->_lvalue;
          }
          break;
        case node_operation::call:
          if (const function_type* ft = std::get_if<function_type>(_children[0]->_type_id)) {
            _type_id = ft->returning;
            _lvalue = false;
            if (ft->has_varargs) {
              if (const array_type* varargs = std::get_if<array_type>(ft->params.back()))
              for (size_t i = 0; i < _children.size() - 1; ++i) {
                if (i >= ft->params.size() - 1)
                  _children[i + 1]->check_conversion(varargs->inner, false);
                else
                  _children[i + 1]->check_conversion(ft->params[i], false);
              } else
                throw compiler_error("varargs not working properly in function signature.", _line_number, _char_index, 1);
            } else {
              if (ft->params.size() != _children.size() - 1) {
                throw semantic_error("expected " + std::to_string(ft->params.size()) + " arguments, got " + std::to_string(_children.size() - 1) + " instead.", _line_number, _char_index, 0);
              }
              for (size_t i = 0; i < ft->params.size(); ++i) {
                _children[i + 1]->check_conversion(ft->params[i], false);
              }
            }
          } else
            throw semantic_error("'" + std::to_string(_children[0]->_type_id) + "' object is not callable.", _line_number, _char_index, 0);
          break;
        case node_operation::list:
          _type_id = _children.back()->get_type_id(); // Type to check against (temporary)
          for (int i = 0; i < int(_children.size()) - 1; ++i) {
            _children[i]->check_conversion(_type_id, false);
          }
          _type_id = context.get_handle(array_type{_type_id});
          _lvalue = false;
      }
    }
  }*/
}

namespace std {
  using namespace valley;

  string to_string(const node_ptr& n) {
    return visit(overloaded{
      [&](int8_t num) -> string {
        return to_string(num);
      },
      [&](int16_t num) -> string {
        return to_string(num);
      },
      [&](int32_t num) -> string {
        return to_string(num);
      },
      [&](int64_t num) -> string {
        return to_string(num);
      },
      [&](float num) -> string {
        return to_string(num);
      },
      [&](double num) -> string {
        return to_string(num);
      },
      [&](bool b) -> string {
        if (b)
          return "true";
        else
          return "false";
      },
      [&](char c) -> string {
        return "'" + to_string(c) + "'";
      },
      [&](const string& str) -> string {
        return '"' + str + '"';
      },
      [&](const identifier& id) -> string {
        return "$" + id.name;
      },
      [&](const declaration& dec) -> string {
        return "(" + string(dec.is_final ? "final " : "") + string(dec.is_static ? "static " : "") + to_string(dec.type_id) + " $" + dec.name + ")";
      },
      [&](node_operation op) -> string {
        switch (op) {
          case node_operation::un_preinc:
            return "(++" + to_string(n->get_children()[0]) + ")";
          case node_operation::un_predec:
            return "(--" + to_string(n->get_children()[0]) + ")";
          case node_operation::un_postinc:
            return "(" + to_string(n->get_children()[0]) + "++)";
          case node_operation::un_postdec:
            return "(" + to_string(n->get_children()[0]) + "--)";
          case node_operation::un_pos:
            return "(+" + to_string(n->get_children()[0]) + ")";
          case node_operation::un_neg:
            return "(-" + to_string(n->get_children()[0]) + ")";
          case node_operation::un_bwnot:
            return "(~" + to_string(n->get_children()[0]) + ")";
          case node_operation::un_lnot:
            return "(!" + to_string(n->get_children()[0]) + ")";
          case node_operation::bin_add:
            return "(" + to_string(n->get_children()[0]) + " + " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_sub:
            return "(" + to_string(n->get_children()[0]) + " - " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_mul:
            return "(" + to_string(n->get_children()[0]) + " * " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_div:
            return "(" + to_string(n->get_children()[0]) + " / " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_mod:
            return "(" + to_string(n->get_children()[0]) + " % " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_pow:
            return "(" + to_string(n->get_children()[0]) + " ** " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_bwand:
            return "(" + to_string(n->get_children()[0]) + " & " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_bwor:
            return "(" + to_string(n->get_children()[0]) + " | " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_bwxor:
            return "(" + to_string(n->get_children()[0]) + " ^ " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_lshift:
            return "(" + to_string(n->get_children()[0]) + ") + " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_rshift:
            return "(" + to_string(n->get_children()[0]) + " >> " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg:
            return "(" + to_string(n->get_children()[0]) + " = " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_add:
            return "(" + to_string(n->get_children()[0]) + " += " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_sub:
            return "(" + to_string(n->get_children()[0]) + " -= " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_mul:
            return "(" + to_string(n->get_children()[0]) + " *= " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_div:
            return "(" + to_string(n->get_children()[0]) + " /= " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_mod:
            return "(" + to_string(n->get_children()[0]) + " %= " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_pow:
            return "(" + to_string(n->get_children()[0]) + " **= " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_and:
            return "(" + to_string(n->get_children()[0]) + " &= " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_or:
            return "(" + to_string(n->get_children()[0]) + " |= " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_xor:
            return "(" + to_string(n->get_children()[0]) + " ^= " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_lshift:
            return "(" + to_string(n->get_children()[0]) + ") += " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_asg_rshift:
            return "(" + to_string(n->get_children()[0]) + " >>= " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_eq:
            return "(" + to_string(n->get_children()[0]) + " == " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_neq:
            return "(" + to_string(n->get_children()[0]) + " != " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_lt:
            return "(" + to_string(n->get_children()[0]) + " < " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_gt:
            return "(" + to_string(n->get_children()[0]) + " > " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_lteq:
            return "(" + to_string(n->get_children()[0]) + " <= " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_gteq:
            return "(" + to_string(n->get_children()[0]) + " >= " + to_string(n->get_children()[1]) + ")";
          case node_operation::comma:
            return "(" + to_string(n->get_children()[0]) + ", " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_land:
            return "(" + to_string(n->get_children()[0]) + " && " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_lor:
            return "(" + to_string(n->get_children()[0]) + " || " + to_string(n->get_children()[1]) + ")";
          case node_operation::bin_lxor:
            return "(" + to_string(n->get_children()[0]) + " ^^ " + to_string(n->get_children()[1]) + ")";
          case node_operation::index:
            return "(" + to_string(n->get_children()[0]) + "[" + to_string(n->get_children()[1]) + "])";
          case node_operation::ternary:
            return "(" + to_string(n->get_children()[0]) + " ? " + to_string(n->get_children()[1]) + " : " + to_string(n->get_children()[2]) + ")";
          case node_operation::call: {
            string str = to_string(n->get_children()[0]) + "(";
            const char* separator = "";
            for (size_t i = 1; i < n->get_children().size(); ++i) {
              str += separator + to_string(n->get_children()[i]);
              separator = ", ";
            }
            return str + ")";
          }
          case node_operation::list: {
            string str = "[";
            const char* separator = "";
            for (size_t i = 0; i < n->get_children().size(); ++i) {
              str += separator + to_string(n->get_children()[i]);
              separator = ", ";
            }
            return str + "]";
          }
        }
      },
      [&](const auto&) -> string {
        return "";
      }
    }, n->get_value());
  }
}