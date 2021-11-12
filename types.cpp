#include "types.hpp"

#include <iostream>

#include "helpers.hpp"

namespace valley {
  bool type_registry::types_less::operator()(const type& t1, const type& t2) const {
    const size_t idx1 = t1.index();
    const size_t idx2 = t2.index();

    if (idx1 != idx2) {
      return idx1 < idx2;
    }
    
    switch(idx1) {
      case 0:
        return std::get<0>(t1) < std::get<0>(t2);
      case 1:
        return std::get<1>(t1).current < std::get<1>(t2).current;
      case 2:
        return std::get<2>(t1).inner < std::get<2>(t2).inner;
      case 3: {
        const function_type& ft1 = std::get<3>(t1);
        const function_type& ft2 = std::get<3>(t2);

        if (ft1.returning != ft2.returning) {
          return ft1.returning < ft2.returning;
        }

        if (ft1.params.size() != ft2.params.size()) {
          return ft1.params.size() < ft2.params.size();
        }

        for (size_t i = 0; i < ft1.params.size(); ++i) {
          if (ft1.params[i] != ft2.params[i]) {
            return ft1.params[i] < ft2.params[i];
          }
        }
      }
    }

    return false;
  }

  type_registry::type_registry() {
  }

  type_handle type_registry::get_handle(const type& t) {
    return std::visit(overloaded{
      [](primitive t) {
        switch (t) {
          case primitive::void_type:
            return type_registry::void_handle();
          case primitive::byte_type:
            return type_registry::byte_handle();
          case primitive::short_type:
            return type_registry::short_handle();
          case primitive::int_type:
            return type_registry::int_handle();
          case primitive::long_type:
            return type_registry::long_handle();
          case primitive::float_type:
            return type_registry::float_handle();
          case primitive::double_type:
            return type_registry::double_handle();
          case primitive::bool_type:
            return type_registry::bool_handle();
          case primitive::char_type:
            return type_registry::char_handle();
          case primitive::str_type:
            return type_registry::str_handle();
        }
      },
      [this](const auto& t) {
        return &(*(_types.insert(t).first));
      }
    }, t);
  }

  type type_registry::void_type = primitive::void_type;
  type type_registry::byte_type = primitive::byte_type;
  type type_registry::short_type = primitive::short_type;
  type type_registry::int_type = primitive::int_type;
  type type_registry::long_type = primitive::long_type;
  type type_registry::float_type = primitive::float_type;
  type type_registry::double_type = primitive::double_type;
  type type_registry::bool_type = primitive::bool_type;
  type type_registry::char_type = primitive::char_type;
  type type_registry::str_type = primitive::str_type;
  type type_registry::any_type_dummy = any_type{&type_registry::void_type};
}

namespace std {
  using namespace valley;
  
  std::string to_string(type_handle t) {
    if (!t)
      return std::string("(no type)");
    return std::visit(overloaded{
      [](const any_type& t) {
        return std::string("any");
      },
      [](primitive t) {
        switch (t) {
          case primitive::void_type:
            return std::string("void");
          case primitive::byte_type:
            return std::string("byte");
          case primitive::short_type:
            return std::string("short");
          case primitive::int_type:
            return std::string("int");
          case primitive::long_type:
            return std::string("long");
          case primitive::float_type:
            return std::string("float");
          case primitive::double_type:
            return std::string("double");
          case primitive::bool_type:
            return std::string("bool");
          case primitive::char_type:
            return std::string("char");
          case primitive::str_type:
            return std::string("str");
          default:
            return std::string("(unknown type)");
        }
      },
      [](const array_type& at) {
        std::string ret = to_string(at.inner);
        ret += "[]";
        return ret;
      },
      [](const function_type& ft) {
        std::string ret = to_string(ft.returning) + "(";
        const char* separator = "";
        for (type_handle param : ft.params) {
          ret += separator + to_string(param);
          separator = ", ";
        }
        if (ft.has_varargs)
          ret += "...";
        return ret + ")";
      }
    }, *t);
  }
}