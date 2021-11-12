#ifndef TYPES_HPP
#define TYPES_HPP

#include <ostream>
#include <set>
#include <variant>
#include <vector>

namespace valley {
  enum struct primitive {
    void_type,
    byte_type,
    short_type,
    int_type,
    long_type,
    float_type,
    double_type,
    bool_type,
    char_type,
    str_type,
  };

  struct any_type;
  struct array_type;
  struct function_type;

  using type = std::variant<primitive, any_type, array_type, function_type>;
  using type_handle = const type*;

  struct any_type {
    type_handle current;
  };

  struct array_type {
    type_handle inner;
  };

  struct function_type {
    type_handle returning;
    std::vector<type_handle> params;
    bool has_varargs;
  };

  class type_registry {
    private:
      struct types_less {
        bool operator()(const type& t1, const type& t2) const;
      };

      std::set<type, types_less> _types;

      static type void_type;
      static type byte_type;
      static type short_type;
      static type int_type;
      static type long_type;
      static type float_type;
      static type double_type;
      static type bool_type;
      static type char_type;
      static type str_type;
      static type any_type_dummy;
    
    public:
      type_registry();

      type_handle get_handle(const type& t);

      static type_handle any_handle() {
        return &any_type_dummy;
      }

      static type_handle void_handle() {
        return &void_type;
      }

      static type_handle byte_handle() {
        return &byte_type;
      }

      static type_handle short_handle() {
        return &short_type;
      }

      static type_handle int_handle() {
        return &int_type;
      }

      static type_handle long_handle() {
        return &long_type;
      }

      static type_handle float_handle() {
        return &float_type;
      }

      static type_handle double_handle() {
        return &double_type;
      }

      static type_handle bool_handle() {
        return &bool_type;
      }

      static type_handle char_handle() {
        return &char_type;
      }

      static type_handle str_handle() {
        return &str_type;
      }
  };
}

namespace std {
  std::string to_string(valley::type_handle t);
}

#endif