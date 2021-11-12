#include "compiler_context.hpp"

#include <iostream>

namespace valley {
  identifier_info::identifier_info(type_handle type_id, size_t index, bool is_global, bool is_final, bool is_static):
  _type_id(type_id), _index(index), _global(is_global), _final(is_final), _static(is_static) {
  }

  type_handle identifier_info::type_id() const {
    return _type_id;
  }

  size_t identifier_info::index() const {
    return _index;
  }

  bool identifier_info::is_global() const {
    return _global;
  }

  bool identifier_info::is_final() const {
    return _final;
  }

  bool identifier_info::is_static() const {
    return _static;
  }

  const identifier_info* identifier_lookup::insert_identifier(std::string name, type_handle type_id, size_t index, bool is_global, bool is_final, bool is_static) {
    return &_identifiers.emplace(std::move(name), identifier_info(type_id, index, is_global, is_final, is_static)).first->second;
  }

  identifier_lookup::~identifier_lookup() {
  }

  size_t identifier_lookup::identifiers_size() const {
    return _identifiers.size();
  }

  const identifier_info* identifier_lookup::find(const std::string& name) const {
    if (auto it = _identifiers.find(name); it != _identifiers.end()) {
      return &it->second;
    } else {
      return nullptr;
    }
  }

  const identifier_info* global_identifier_lookup::create_identifier(std::string name, type_handle type_id, bool is_final, bool is_static) {
    return insert_identifier(std::move(name), type_id, identifiers_size(), true, is_final, is_static);
  }

  local_identifier_lookup::local_identifier_lookup(std::unique_ptr<local_identifier_lookup> parent_lookup):
  _parent(std::move(parent_lookup)), _next_index(_parent ? _parent->_next_index : 1) {
  }

  const identifier_info* local_identifier_lookup::find(const std::string& name) const {
    if (const identifier_info* ret = identifier_lookup::find(name)) {
      return ret;
    } else {
      return _parent ? _parent->find(name) : nullptr;
    }
  }

  const identifier_info* local_identifier_lookup::create_identifier(std::string name, type_handle type_id, bool is_final, bool is_static) {
    return insert_identifier(std::move(name), type_id, _next_index++, false, is_final, is_static);
  }

  std::unique_ptr<local_identifier_lookup> local_identifier_lookup::detach_parent() {
    return std::move(_parent);
  }

  function_identifier_lookup::function_identifier_lookup(): local_identifier_lookup(nullptr), _next_param_index(-1) {
  }

  const identifier_info* function_identifier_lookup::create_param(std::string name, type_handle type_id) {
    return insert_identifier(std::move(name), type_id, _next_param_index--, false, false, false);
  }

  compiler_context::compiler_context(): _params(nullptr) {
  }

  type_handle compiler_context::get_handle(const type& t) {
    return _types.get_handle(t);
  }

  const identifier_info* compiler_context::find(const std::string& name) const {
    if (_locals) {
      if (const identifier_info* ret = _locals->find(name)) {
        return ret;
      }
    }
    return _globals.find(name);
  }

  const identifier_info* compiler_context::create_identifier(std::string name, type_handle type_id, bool is_final, bool is_static) {
    if (_locals) {
      return _locals->create_identifier(std::move(name), type_id, is_final, is_static);
    } else {
      return _globals.create_identifier(std::move(name), type_id, is_final, is_static);
    }
  }

  const identifier_info* compiler_context::create_param(std::string name, type_handle type_id) {
    return _params->create_param(name, type_id);
  }

  void compiler_context::enter_scope() {
    _locals = std::make_unique<local_identifier_lookup>(std::move(_locals));
  }

  void compiler_context::enter_function() {
    std::unique_ptr<function_identifier_lookup> params = std::make_unique<function_identifier_lookup>();
    _params = params.get();
    _locals = std::move(params);
  }

  bool compiler_context::leave_scope() {
    if (!_locals) {
      return false;
    }
    if (_params == _locals.get()) {
      _params = nullptr;
    }
    _locals = _locals->detach_parent();
    return true;
  }
}