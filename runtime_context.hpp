#ifndef RUNTIME_CONTEXT_HPP
#define RUNTIME_CONTEXT_HPP

#include <deque>
#include <unordered_map>

#include "variable.hpp"

namespace valley {
  class runtime_context {
    private:
      std::unordered_map<std::string, variable_ptr> _heap;
      std::deque<variable_ptr> _stack_vars;
      std::deque<std::string> _stack_keys;
      std::deque<size_t> _scope_sizes;
    
    public:
      runtime_context(size_t heap_size);

      variable_ptr find(const std::string& name) const;
      void add_stack(std::string name, variable_ptr var);
      void add_heap(std::string name, variable_ptr var);

      void enter_scope();
      bool leave_scope();
      variable_ptr leave_function();
  };
}

#endif