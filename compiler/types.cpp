#include "types.hpp"

#include "util.hpp"


namespace valley {

  // Returns true if t1 has a smaller index in the Type union than t2
  bool TypeRegistry::TypeComparator::operator()(const Type& t1, const Type& t2) const {
    const size_t idx1 = t1.index();
    const size_t idx2 = t2.index();

    if (idx1 != idx2) {
      return idx1 < idx2;
    }
    
    switch (idx1) {
      case 0: // PrimitiveType
        return std::get<0>(t1) < std::get<0>(t2);

      case 1: // ArrayType
        return std::get<1>(t1).inner < std::get<1>(t2).inner;

      case 2: //FuncType
      {
        const FuncType& ft1 = std::get<2>(t1);
        const FuncType& ft2 = std::get<2>(t2);

        if (ft1.returnType != ft2.returnType)
          return ft1.returnType < ft2.returnType;

        if (ft1.paramTypes.size() != ft2.paramTypes.size())
          return ft1.paramTypes.size() < ft2.paramTypes.size();

        for (size_t i = 0; i < ft1.paramTypes.size(); ++i) {
          if (ft1.paramTypes[i] != ft2.paramTypes[i])
            return ft1.paramTypes[i] < ft2.paramTypes[i];
        }
      }

      case 3: // AnyType
        return std::get<3>(t1).active < std::get<3>(t2).active;
      
      case 4: // ClassType
        return std::get<4>(t1).className.compare(std::get<4>(t2).className) < 0;
      
      case 5: // ObjectType
        return std::get<5>(t1).classType->className.compare(std::get<5>(t2).classType->className) < 0;
    }

    return false;
  }

  TypeRegistry::TypeRegistry() {
  }

  TypeHandle TypeRegistry::getHandle(const Type& type) {
    return std::visit(Overloaded{
      [](PrimitiveType t) {
        switch (t) {
          case PrimitiveType::VOID:
            return TypeRegistry::voidHandle();

          case PrimitiveType::BYTE:
            return TypeRegistry::byteHandle();

          case PrimitiveType::SHORT:
            return TypeRegistry::shortHandle();

          case PrimitiveType::INT:
            return TypeRegistry::intHandle();

          case PrimitiveType::LONG:
            return TypeRegistry::longHandle();

          case PrimitiveType::FLOAT:
            return TypeRegistry::floatHandle();

          case PrimitiveType::DOUBLE:
            return TypeRegistry::doubleHandle();

          case PrimitiveType::BOOL:
            return TypeRegistry::boolHandle();

          case PrimitiveType::CHAR:
            return TypeRegistry::charHandle();

          case PrimitiveType::STR:
            return TypeRegistry::strHandle();
        }
      },

      [this](const auto& t) {
        // If the handle doesn't exist, register it
        // Either way, return the handle corresponding to the type
        return &(*(_register.insert(t).first));
      }
    }, type);
  }

  Type TypeRegistry::_void_ref   = PrimitiveType::VOID;
  Type TypeRegistry::_byte_ref   = PrimitiveType::BYTE;
  Type TypeRegistry::_short_ref  = PrimitiveType::SHORT;
  Type TypeRegistry::_int_ref    = PrimitiveType::INT;
  Type TypeRegistry::_long_ref   = PrimitiveType::LONG;
  Type TypeRegistry::_float_ref  = PrimitiveType::FLOAT;
  Type TypeRegistry::_double_ref = PrimitiveType::DOUBLE;
  Type TypeRegistry::_bool_ref   = PrimitiveType::BOOL;
  Type TypeRegistry::_char_ref   = PrimitiveType::CHAR;
  Type TypeRegistry::_str_ref    = PrimitiveType::STR;
  Type TypeRegistry::_any_ref    = AnyType{nullptr};

  std::string typeHandleRepr(TypeHandle type) {
    if (!type)
      return std::string("<undefined type>");
    
    return std::visit(Overloaded{
      [](const AnyType& t) {
        return std::string("any");
      },
      [](PrimitiveType t) {
        switch (t) {
          case PrimitiveType::VOID:
            return std::string("void");

          case PrimitiveType::BYTE:
            return std::string("byte");

          case PrimitiveType::SHORT:
            return std::string("short");

          case PrimitiveType::INT:
            return std::string("int");

          case PrimitiveType::LONG:
            return std::string("long");

          case PrimitiveType::FLOAT:
            return std::string("float");

          case PrimitiveType::DOUBLE:
            return std::string("double");

          case PrimitiveType::BOOL:
            return std::string("bool");

          case PrimitiveType::CHAR:
            return std::string("char");

          case PrimitiveType::STR:
            return std::string("str");
        }
      },
      [](const ArrayType& t) {
        std::string ret = typeHandleRepr(t.inner);
        ret += "[]";
        return ret;
      },
      [](const FuncType& t) {
        std::string ret = typeHandleRepr(t.returnType) + "(";
        const char* separator = "";
        for (TypeHandle param : t.paramTypes) {
          ret += separator + typeHandleRepr(param);
          separator = ", ";
        }
        if (t.hasArgCatch)
          ret += "...";
        return ret + ")";
      },
      [](const ClassType& t) {
        return std::string("class");
      },
      [](const ObjectType& t) {
        return t.classType->className;
      }
    }, *type);
  }

}