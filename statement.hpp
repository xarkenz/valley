#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include "compiler_context.hpp"
#include "expression_manager.hpp"
#include "runtime_context.hpp"

namespace valley {
  enum struct statement_type {
    empty_s,
    expr_s,
    block_s,
    declare_s,
    decfunc_s,
    return_s,
    break_s,
    continue_s,
    ifelse_s,
    while_s,
    dowhile_s,
    for_s,
    foreach_s,
    switch_s,
    case_s,
    trycatch_s,
  };
  
  class statement: std::enable_shared_from_this<statement> {
    private:
      statement(const statement&) = delete;
      void operator=(const statement&) = delete;
    
    public:
      using ptr = std::shared_ptr<const statement>;

      virtual ~statement() = default;
      virtual const statement_type get_stmt_type() const = 0;

      std::weak_ptr<const statement> get_parent() const;
      size_t get_line_number() const;
      size_t get_char_index() const;
      void set_parent(ptr parent) const;

      bool execute(runtime_context& context) const;
      void do_return(runtime_context& context) const;
      void do_break(runtime_context& context) const;
      void do_continue(runtime_context& context) const;

      virtual std::string to_string() const = 0;
    
    protected:
      statement(ptr parent, size_t line_number, size_t char_index);

      mutable std::weak_ptr<const statement> _parent;
      size_t _line_number;
      size_t _char_index;
  };

  // Empty statement
  class statement_empty: public statement {
    public:
      statement_empty(statement::ptr parent, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      std::string to_string() const;
  };

  // <root: <expr>>
  class statement_expr: public statement {
    private:
      node_ptr _root;

    public:
      statement_expr(statement::ptr parent, node_ptr& root, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      const node_ptr& get_root() const;

      bool execute(runtime_context& context) const;

      std::string to_string() const;
  };

  // {<contents...>}
  class statement_block: public statement {
    private:
      std::vector<statement::ptr> _contents;
    
    public:
      statement_block(statement::ptr parent, std::vector<statement::ptr> contents, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      std::vector<statement::ptr>& get_contents();

      bool execute(runtime_context& context) const;

      std::string to_string() const;
  };

  // <id_info> <id_name> [= <value>]
  class statement_declare: public statement {
    private:
      const identifier_info _id_info;
      const std::string _id_name;
      statement::ptr _value;
    
    public:
      statement_declare(statement::ptr parent, const identifier_info* id_info, const std::string& id_name, statement::ptr value, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      const identifier_info* get_id_info() const;
      const std::string& get_id_name() const;
      statement::ptr get_value() const;

      bool execute(runtime_context& context) const;

      std::string to_string() const;
  };

  // <id_info> <id_name>(<info> <name>, ...) [<exec>]
  class statement_decfunc: public statement {
    private:
      const identifier_info _id_info;
      const std::string _id_name;
      std::vector<identifier_info> _param_infos;
      std::vector<std::string> _param_names;
      statement::ptr _exec;
    
    public:
      statement_decfunc(statement::ptr parent, const identifier_info* id_info, const std::string& id_name, std::vector<identifier_info> param_infos, std::vector<std::string> param_names, statement::ptr exec, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      const identifier_info* get_id_info() const;
      const std::string& get_id_name() const;
      const std::vector<identifier_info>& get_param_infos() const;
      const std::vector<std::string>& get_param_names() const;
      statement::ptr get_exec() const;

      bool execute(runtime_context& context) const;
      void do_return(runtime_context& context) const;
      
      std::string to_string() const;
  };

  // return [<value>]
  class statement_return: public statement {
    private:
      statement::ptr _value;
    
    public:
      statement_return(statement::ptr parent, statement::ptr value, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      statement::ptr get_value() const;

      bool execute(runtime_context& context) const;

      std::string to_string() const;
  };

  // break
  class statement_break: public statement {
    public:
      statement_break(statement::ptr parent, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      bool execute(runtime_context& context) const;

      std::string to_string() const;
  };

  // continue
  class statement_continue: public statement {
    public:
      statement_continue(statement::ptr parent, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      bool execute(runtime_context& context) const;

      std::string to_string() const;
  };

  // if (<condition>) <do_if> [else <do_else>]
  class statement_if_else: public statement {
    private:
      statement::ptr _condition;
      statement::ptr _do_if;
      statement::ptr _do_else;
    
    public:
      statement_if_else(statement::ptr parent, statement::ptr condition, statement::ptr do_if, statement::ptr do_else, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      statement::ptr get_condition() const;
      statement::ptr get_do_if() const;
      statement::ptr get_do_else() const;

      bool execute(runtime_context& context) const;

      std::string to_string() const;
  };

  // while (<condition>) <looped>
  class statement_while: public statement {
    protected:
      statement::ptr _condition;
      statement::ptr _looped;
    
    public:
      statement_while(statement::ptr parent, statement::ptr condition, statement::ptr looped, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      statement::ptr get_condition() const;
      statement::ptr get_looped() const;

      bool execute(runtime_context& context) const;
      void do_break(runtime_context& context) const;
      void do_continue(runtime_context& context) const;

      std::string to_string() const;
  };

  // do <looped> while (<condition>);
  class statement_do_while: public statement_while {
    public:
      statement_do_while(statement::ptr parent, statement::ptr condition, statement::ptr looped, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const override;

      bool execute(runtime_context& context) const;
      void do_break(runtime_context& context) const;
      void do_continue(runtime_context& context) const;
      
      std::string to_string() const override;
  };

  // for ([<first>]; [<condition>]; [<on_iter>]) <looped>
  class statement_for: public statement {
    private:
      statement::ptr _first;
      statement::ptr _condition;
      statement::ptr _on_iter;
      statement::ptr _looped;
    
    public:
      statement_for(statement::ptr parent, statement::ptr first, statement::ptr condition, statement::ptr on_iter, statement::ptr looped, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      statement::ptr get_first() const;
      statement::ptr get_condition() const;
      statement::ptr get_on_iter() const;
      statement::ptr get_looped() const;

      bool execute(runtime_context& context) const;
      void do_break(runtime_context& context) const;
      void do_continue(runtime_context& context) const;

      std::string to_string() const;
  };

  // for (<declared> : <iter>) <looped>
  class statement_foreach: public statement {
    private:
      statement::ptr _declared;
      statement::ptr _iter;
      statement::ptr _looped;
    
    public:
      statement_foreach(statement::ptr parent, statement::ptr declared, statement::ptr iter, statement::ptr looped, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      statement::ptr get_declared() const;
      statement::ptr get_iter() const;
      statement::ptr get_looped() const;

      bool execute(runtime_context& context) const;
      void do_break(runtime_context& context) const;
      void do_continue(runtime_context& context) const;

      std::string to_string() const;
  };

  // switch (<tested>) <contents>
  class statement_switch: public statement {
    private:
      statement::ptr _tested;
      statement::ptr _contents;
    
    public:
      statement_switch(statement::ptr parent, statement::ptr tested, statement::ptr contents, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      statement::ptr get_tested() const;
      statement::ptr get_contents() const;

      bool execute(runtime_context& context) const;
      void do_break(runtime_context& context) const;
      void do_continue(runtime_context& context) const;

      std::string to_string() const;
  };

  // case [<test>]: | default:
  class statement_switch_case: public statement {
    private:
      node_ptr& _test;
    
    public:
      statement_switch_case(statement::ptr parent, node_ptr& test, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      node_ptr& get_test() const;

      bool execute(runtime_context& context) const;

      std::string to_string() const;
  };

  // (catch [(<except_type> [<except_id>])] [<on_catch>])
  struct catcher_info {
    type_handle except_type;
    const std::string except_id;
    statement::ptr on_catch;
  };
  
  // try <to_try> [catch (<except_type> <except_id>) <on_catch> [catch ...]] [finally <do_after>]
  class statement_try_catch: public statement {
    private:
      statement::ptr _to_try;
      std::vector<catcher_info> _catchers;
      statement::ptr _do_after;
    
    public:
      statement_try_catch(statement::ptr parent, statement::ptr to_try, std::vector<catcher_info>& catchers, statement::ptr do_after, size_t line_number, size_t char_index);
      const statement_type get_stmt_type() const;

      statement::ptr get_to_try() const;
      const catcher_info* find_catcher(type_handle except_type) const;
      statement::ptr get_do_after() const;

      bool execute(runtime_context& context) const;

      std::string to_string() const;
  };
}

#endif