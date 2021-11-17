#pragma once

#include <set>
#include <variant>
#include <vector>


namespace valley {

  // Type index 0 (fundamental built-in types)
  enum struct PrimitiveType {
    VOID,
    BYTE,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
    BOOL,
    CHAR,
    STR,
  };

  struct ArrayType;
  struct FuncType;
  struct AnyType;
  struct ClassType;
  struct ObjectType;

  using Type = std::variant<PrimitiveType, ArrayType, FuncType, AnyType, ClassType, ObjectType>;
  // Allows objects to hold globally constant types
  using TypeHandle = const Type*;

  // Type index 1 (arrays of a certain type)
  struct ArrayType {
    TypeHandle inner;
  };

  // Type index 2 (defined function with parameters and return)
  struct FuncType {
    TypeHandle returnType;
    std::vector<TypeHandle> paramTypes;
    bool hasArgCatch; // true if last parameter has '...'
  };

  // Type index 3 (dynamic type; active type is nullptr if n/a, e.g. any[])
  struct AnyType {
    TypeHandle active;
  };

  // Type index 4 (user-defined classes; name is constant and unique)
  struct ClassType {
    std::string className;
    // Same as method resolution order, useful when checking implicit conversion and matching function calls to signatures
    std::vector<const ClassType*> inheritance;
  };

  // Type index 5 (instances of user-defined classes)
  struct ObjectType {
    const ClassType* classType;
  };

  class TypeRegistry {
  private:
    // Compares types in the register
    struct TypeComparator {
      bool operator()(const Type& t1, const Type& t2) const;
    };

    // Internal register
    std::set<Type, TypeComparator> _register;

    // Static types to hold the basic registry types
    static Type _void_ref;
    static Type _byte_ref;
    static Type _short_ref;
    static Type _int_ref;
    static Type _long_ref;
    static Type _float_ref;
    static Type _double_ref;
    static Type _bool_ref;
    static Type _char_ref;
    static Type _str_ref;
    static Type _any_ref;
  
  public:
    TypeRegistry();

    // Register/Get a type handle
    TypeHandle getHandle(const Type& t);

    // Getters for the basic registry types above
    static TypeHandle voidHandle() { return &_void_ref; }
    static TypeHandle byteHandle() { return &_byte_ref; }
    static TypeHandle shortHandle() { return &_short_ref; }
    static TypeHandle intHandle() { return &_int_ref; }
    static TypeHandle longHandle() { return &_long_ref; }
    static TypeHandle floatHandle() { return &_float_ref; }
    static TypeHandle doubleHandle() { return &_double_ref; }
    static TypeHandle boolHandle() { return &_bool_ref; }
    static TypeHandle charHandle() { return &_char_ref; }
    static TypeHandle strHandle() { return &_str_ref; }
    static TypeHandle anyHandle() { return &_any_ref; }
  };

  std::string typeHandleRepr(TypeHandle type);

}