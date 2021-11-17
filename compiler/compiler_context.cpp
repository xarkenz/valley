#include "compiler_context.hpp"


namespace valley {

  IdentifierInfo::IdentifierInfo(TypeHandle type, size_t index, bool is_global, bool is_final, bool is_static):
    _type(type),
    _index(index),
    _global(is_global),
    _final(is_final),
    _static(is_static)
  {
  }

  TypeHandle IdentifierInfo::type() const {
    return _type;
  }

  size_t IdentifierInfo::index() const {
    return _index;
  }

  bool IdentifierInfo::isGlobal() const {
    return _global;
  }

  bool IdentifierInfo::isFinal() const {
    return _final;
  }

  bool IdentifierInfo::isStatic() const {
    return _static;
  }

  const IdentifierInfo* IdentifierLookup::insertIdentifier(std::string name, TypeHandle type, size_t index, bool isGlobal, bool isFinal, bool isStatic) {
    return &_identifiers.emplace(std::move(name), IdentifierInfo(type, index, isGlobal, isFinal, isStatic)).first->second;
  }

  IdentifierLookup::~IdentifierLookup() {
  }

  size_t IdentifierLookup::size() const {
    return _identifiers.size();
  }

  const IdentifierInfo* IdentifierLookup::find(const std::string& name) const {
    if (auto it = _identifiers.find(name); it != _identifiers.end())
      return &it->second;
    return nullptr;
  }

  const IdentifierInfo* GlobalIdentifierLookup::createIdentifier(std::string name, TypeHandle type, bool isFinal, bool isStatic) {
    return insertIdentifier(std::move(name), type, size(), true, isFinal, isStatic);
  }

  LocalIdentifierLookup::LocalIdentifierLookup(std::unique_ptr<LocalIdentifierLookup> parent):
    _parent(std::move(parent)),
    _next_index(_parent ? _parent->_next_index : 1)
  {
  }

  const IdentifierInfo* LocalIdentifierLookup::find(const std::string& name) const {
    if (const IdentifierInfo* ret = IdentifierLookup::find(name))
      return ret;
    return _parent ? _parent->find(name) : nullptr;
  }

  const IdentifierInfo* LocalIdentifierLookup::createIdentifier(std::string name, TypeHandle type, bool isFinal, bool isStatic) {
    return insertIdentifier(std::move(name), type, _next_index++, false, isFinal, isStatic);
  }

  std::unique_ptr<LocalIdentifierLookup> LocalIdentifierLookup::detachParent() {
    return std::move(_parent);
  }

  FunctionIdentifierLookup::FunctionIdentifierLookup():
    LocalIdentifierLookup(nullptr),
    _next_param_index(-1) {
  }

  const IdentifierInfo* FunctionIdentifierLookup::createParam(std::string name, TypeHandle type) {
    return insertIdentifier(std::move(name), type, _next_param_index--, false, false, false);
  }

  CompilerContext::CompilerContext():
    _params(nullptr)
  {
  }

  TypeHandle CompilerContext::getHandle(const Type& t) {
    return _types.getHandle(t);
  }

  const IdentifierInfo* CompilerContext::find(const std::string& name) const {
    if (_locals)
      if (const IdentifierInfo* ret = _locals->find(name))
        return ret;
    return _globals.find(name);
  }

  const IdentifierInfo* CompilerContext::createIdentifier(std::string name, TypeHandle type, bool isFinal, bool isStatic) {
    if (_locals)
      return _locals->createIdentifier(std::move(name), type, isFinal, isStatic);
    return _globals.createIdentifier(std::move(name), type, isFinal, isStatic);
  }

  const IdentifierInfo* CompilerContext::createParam(std::string name, TypeHandle type) {
    return _params->createParam(name, type);
  }

  void CompilerContext::enterScope() {
    _locals = std::make_unique<LocalIdentifierLookup>(std::move(_locals));
  }

  void CompilerContext::enterFunction() {
    std::unique_ptr<FunctionIdentifierLookup> params = std::make_unique<FunctionIdentifierLookup>();
    _params = params.get();
    _locals = std::move(params);
  }

  bool CompilerContext::leaveScope() {
    if (!_locals)
      return false;
    if (_params == _locals.get())
      _params = nullptr;
    _locals = _locals->detachParent();
    return true;
  }
  
}