#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <variant>

#include "tokens.hpp"

namespace valley {
  class variable;
  using variable_ptr = std::shared_ptr<variable>;
  template <typename T> class variable_impl;
  class runtime_context;

  using var_void = void_value;
  using var_byte = int8_t;
  using var_short = int16_t;
  using var_int = int32_t;
  using var_long = int64_t;
  using var_float = float;
  using var_double = double;
  using var_bool = bool;
  using var_char = char;
  using var_str = std::shared_ptr<std::string>;
  using var_array = std::deque<variable_ptr>;
  using var_func = std::function<void(runtime_context&)>;

  using lvar_void = std::shared_ptr<variable_impl<var_void>>;
  using lvar_byte = std::shared_ptr<variable_impl<var_byte>>;
  using lvar_short = std::shared_ptr<variable_impl<var_short>>;
  using lvar_int = std::shared_ptr<variable_impl<var_int>>;
  using lvar_long = std::shared_ptr<variable_impl<var_long>>;
  using lvar_float = std::shared_ptr<variable_impl<var_float>>;
  using lvar_double = std::shared_ptr<variable_impl<var_double>>;
  using lvar_bool = std::shared_ptr<variable_impl<var_bool>>;
  using lvar_char = std::shared_ptr<variable_impl<var_char>>;
  using lvar_str = std::shared_ptr<variable_impl<var_str>>;
  using lvar_array = std::shared_ptr<variable_impl<var_array>>;
  using lvar_func = std::shared_ptr<variable_impl<var_func>>;

  using lvar_value = variable_ptr;
  using rvar_value = std::variant<var_void, var_byte, var_short, var_int, var_long, var_float, var_double, var_bool, var_char, var_str, var_array, var_func>;
  using var_value = std::variant<var_void, var_byte, var_short, var_int, var_long, var_float, var_double, var_bool, var_char, var_str, var_array, var_func, lvar_value>;

  class variable: public std::enable_shared_from_this<variable> {
    private:
      variable(const variable&) = delete;
      void operator=(const variable&) = delete;
    
    protected:
      variable() = default;
    
    public:
      virtual ~variable() = default;

      template <typename T>
      T static_downcast() {
        return std::static_pointer_cast<variable_impl<typename T::element_type::value_type>>(shared_from_this());
      }

      auto dynamic_downcast() {
        // return std::static_pointer_cast<shared_from_this()::element_type>(shared_from_this());
        return shared_from_this();
      }

      virtual variable_ptr clone() const = 0;
  };

  template <typename T>
  class variable_impl: public variable {
    public:
      using value_type = T;

      value_type value;

      variable_impl(value_type value);

      variable_ptr clone() const override;
  };

  var_byte clone_var_value(var_byte value);
  var_short clone_var_value(var_short value);
  var_int clone_var_value(var_int value);
  var_long clone_var_value(var_long value);
  var_float clone_var_value(var_float value);
  var_double clone_var_value(var_double value);
  var_bool clone_var_value(var_bool value);
  var_char clone_var_value(var_char value);
  var_str clone_var_value(const var_str& value);
  var_array clone_var_value(const var_array& value);
  var_func clone_var_value(const var_func& value);

  template <class T>
  T clone_var_value(const std::shared_ptr<variable_impl<T>>& v) {
    return clone_var_value(v->value);
  }
}

#endif
