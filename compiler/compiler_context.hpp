#pragma once

#include <memory>
#include <unordered_map>

#include "types.hpp"


namespace valley {

  class IdentifierInfo {
  private:
    TypeHandle _type;
    size_t _index;
    bool _global;
    bool _final;
    bool _static;

  public:
    IdentifierInfo(TypeHandle type, size_t index, bool isGlobal, bool isFinal, bool isStatic);

    TypeHandle type() const;
    size_t index() const;
    bool isGlobal() const;
    bool isFinal() const;
    bool isStatic() const;
  };

  class IdentifierLookup {
  private:
    std::unordered_map<std::string, IdentifierInfo> _identifiers;
  
  protected:
    const IdentifierInfo* insertIdentifier(std::string name, TypeHandle type, size_t index, bool isGlobal, bool isFinal, bool isStatic);
    size_t size() const;

  public:
    virtual ~IdentifierLookup();

    virtual const IdentifierInfo* find(const std::string& name) const;
    virtual const IdentifierInfo* createIdentifier(std::string name, TypeHandle type, bool isFinal, bool isStatic) = 0;
  };

  class GlobalIdentifierLookup: public IdentifierLookup {
  public:
    const IdentifierInfo* createIdentifier(std::string name, TypeHandle type, bool isFinal, bool isStatic) override;
  };

  class LocalIdentifierLookup: public IdentifierLookup {
  private:
    std::unique_ptr<LocalIdentifierLookup> _parent;
    size_t _next_index;

  public:
    LocalIdentifierLookup(std::unique_ptr<LocalIdentifierLookup> parent);

    const IdentifierInfo* find(const std::string& name) const override;
    const IdentifierInfo* createIdentifier(std::string name, TypeHandle type, bool isFinal, bool isStatic) override;
    std::unique_ptr<LocalIdentifierLookup> detachParent();
  };

  class FunctionIdentifierLookup: public LocalIdentifierLookup {
  private:
    size_t _next_param_index;
  
  public:
    FunctionIdentifierLookup();

    const IdentifierInfo* createParam(std::string name, TypeHandle type);
  };

  class CompilerContext {
  private:
    GlobalIdentifierLookup _globals;
    std::unique_ptr<LocalIdentifierLookup> _locals;
    FunctionIdentifierLookup* _params;
    TypeRegistry _types;
  
  public:
    CompilerContext();

    TypeHandle getHandle(const Type& t);
    const IdentifierInfo* find(const std::string& name) const;
    const IdentifierInfo* createIdentifier(std::string name, TypeHandle type, bool isFinal, bool isStatic);
    const IdentifierInfo* createParam(std::string name, TypeHandle type);

    void enterScope();
    void enterFunction();
    bool leaveScope();
  };

}