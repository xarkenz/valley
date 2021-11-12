#ifndef COMPILER_CONTEXT_HPP
#define COMPILER_CONTEXT_HPP

#include <memory>
#include <unordered_map>

#include "types.hpp"

namespace valley {
  class identifier_info {
    private:
      type_handle _type_id;
      size_t _index;
      bool _global;
      bool _final;
      bool _static;

    public:
      identifier_info(type_handle type_id, size_t index, bool is_global, bool is_final, bool is_static);

      type_handle type_id() const;
      size_t index() const;
      bool is_global() const;
      bool is_final() const;
      bool is_static() const;
  };

  class identifier_lookup {
    private:
      std::unordered_map<std::string, identifier_info> _identifiers;
    
    protected:
      const identifier_info* insert_identifier(std::string name, type_handle type_id, size_t index, bool is_global, bool is_final, bool is_static);
      size_t identifiers_size() const;

    public:
      virtual ~identifier_lookup();

      virtual const identifier_info* find(const std::string& name) const;
      virtual const identifier_info* create_identifier(std::string name, type_handle type_id, bool is_final, bool is_static) = 0;
  };

  class global_identifier_lookup: public identifier_lookup {
    public:
      const identifier_info* create_identifier(std::string name, type_handle type_id, bool is_final, bool is_static) override;
  };

  class local_identifier_lookup: public identifier_lookup {
    private:
      std::unique_ptr<local_identifier_lookup> _parent;
      int _next_index;

    public:
      local_identifier_lookup(std::unique_ptr<local_identifier_lookup> parent_lookup);

      const identifier_info* find(const std::string& name) const override;
      const identifier_info* create_identifier(std::string name, type_handle type_id, bool is_final, bool is_static) override;
      std::unique_ptr<local_identifier_lookup> detach_parent();
  };

  class function_identifier_lookup: public local_identifier_lookup {
    private:
      int _next_param_index;
    
    public:
      function_identifier_lookup();

      const identifier_info* create_param(std::string name, type_handle type_id);
  };

  class compiler_context {
    private:
      global_identifier_lookup _globals;
      std::unique_ptr<local_identifier_lookup> _locals;
      function_identifier_lookup* _params;
      type_registry _types;
    
    public:
      compiler_context();

      type_handle get_handle(const type& t);
      const identifier_info* find(const std::string& name) const;
      const identifier_info* create_identifier(std::string name, type_handle type_id, bool is_final, bool is_static);
      const identifier_info* create_param(std::string name, type_handle type_id);

      void enter_scope();
      void enter_function();
      bool leave_scope();
  };
}

#endif
