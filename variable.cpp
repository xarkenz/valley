#include "variable.hpp"

namespace valley {
  template <typename T>
  variable_impl<T>::variable_impl(T value): value(std::move(value)) {
  }

  template <typename T>
  variable_ptr variable_impl<T>::clone() const {
    return std::make_shared<variable_impl<T>>(clone_var_value(value));
  }

  template class variable_impl<var_byte>;
  template class variable_impl<var_short>;
  template class variable_impl<var_int>;
  template class variable_impl<var_long>;
  template class variable_impl<var_float>;
  template class variable_impl<var_double>;
  template class variable_impl<var_bool>;
  template class variable_impl<var_char>;
  template class variable_impl<var_str>;
  template class variable_impl<var_array>;
  template class variable_impl<var_func>;

  var_byte clone_var_value(var_byte value) {
    return value;
  }

  var_short clone_var_value(var_short value) {
    return value;
  }

  var_int clone_var_value(var_int value) {
    return value;
  }

  var_long clone_var_value(var_long value) {
    return value;
  }

  var_float clone_var_value(var_float value) {
    return value;
  }

  var_double clone_var_value(var_double value) {
    return value;
  }

  var_bool clone_var_value(var_bool value) {
    return value;
  }

  var_char clone_var_value(var_char value) {
    return value;
  }

  var_str clone_var_value(const var_str& value) {
    return value;
  }

  var_array clone_var_value(const var_array& value) {
    var_array ret;
    for (const variable_ptr& v : value) {
      ret.push_back(v->clone());
    }
    return ret;
  }

  var_func clone_var_value(const var_func& value) {
    return value;
  }
}