#include "runtime_context.hpp"

namespace valley {
  runtime_context::runtime_context(size_t heap_size): _heap(heap_size), _stack_vars(), _stack_keys(), _scope_sizes() {
    _scope_sizes.push_back(0);
  }

  variable_ptr runtime_context::find(const std::string& name) const {
    for (size_t idx = 0; idx < _stack_keys.size(); ++idx) {
      if (_stack_keys[idx] == name)
        return _stack_vars[idx];
    }
    return nullptr;
  }

  void runtime_context::add_stack(std::string name, variable_ptr var) {
    _stack_keys.push_back(std::move(name));
    _stack_vars.push_back(std::move(var));
  }

  void runtime_context::add_heap(std::string name, variable_ptr var) {
    _heap.insert({std::move(name), std::move(var)});
  }

  void runtime_context::enter_scope() {
    _scope_sizes.push_back(0);
  }

  bool runtime_context::leave_scope() {
    _stack_keys.resize(_stack_keys.size() - _scope_sizes.back());
    _stack_vars.resize(_stack_keys.size());
    _scope_sizes.pop_back();
    return !_scope_sizes.empty();
  }

  variable_ptr runtime_context::leave_function() {
    variable_ptr ret = std::move(_stack_vars.back());
    leave_scope();
    return ret;
  }
}