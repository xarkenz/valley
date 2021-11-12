#include "statement.hpp"

#include <iostream>

#include "errors.hpp"

namespace valley {
  // --- statement ---

  statement::statement(statement::ptr parent, size_t line_number, size_t char_index):
  _parent(parent), _line_number(line_number), _char_index(char_index) {
  }

  std::weak_ptr<const statement> statement::get_parent() const {
    return _parent;
  }

  size_t statement::get_line_number() const {
    return _line_number;
  }

  size_t statement::get_char_index() const {
    return _char_index;
  }

  void statement::set_parent(statement::ptr parent) const {
    _parent = parent;
  }

  bool statement::execute(runtime_context& context) const {
    // Proceed normally by default
    return true;
  }

  void statement::do_return(runtime_context& context) const {
    if (statement::ptr parent = _parent.lock())
      parent->do_return(context);
    else
      throw runtime_error("'return' called outside function definition.", _line_number, _char_index);
  }

  void statement::do_break(runtime_context& context) const {
    if (statement::ptr parent = _parent.lock())
      parent->do_break(context);
    else
      throw runtime_error("'break' called outside loop or switch.", _line_number, _char_index);
  }

  void statement::do_continue(runtime_context& context) const {
    if (statement::ptr parent = _parent.lock())
      parent->do_continue(context);
    else
      throw runtime_error("'continue' called outside loop or switch.", _line_number, _char_index);
  }

  // --- statement_empty ---

  statement_empty::statement_empty(statement::ptr parent, size_t line_number, size_t char_index):
  statement(parent, line_number, char_index) {
  }

  const statement_type statement_empty::get_stmt_type() const {
    return statement_type::empty_s;
  }

  std::string statement_empty::to_string() const {
    return "<EMPTY>";
  }

  // --- statement_expr ---

  statement_expr::statement_expr(statement::ptr parent, node_ptr& root, size_t line_number, size_t char_index):
  _root(std::move(root)), statement(parent, line_number, char_index) {
  }

  const statement_type statement_expr::get_stmt_type() const {
    return statement_type::expr_s;
  }

  const node_ptr& statement_expr::get_root() const {
    return _root;
  }

  bool statement_expr::execute(runtime_context& context) const {
    return true;
  }

  std::string statement_expr::to_string() const {
    if (!_root)
      return "(deallocated)";
    return "<EXPR [" + std::to_string(_root) + "]>";
  }

  // --- statement_block ---

  statement_block::statement_block(statement::ptr parent, std::vector<statement::ptr> contents, size_t line_number, size_t char_index):
  _contents(std::move(contents)), statement(parent, line_number, char_index) {
  }

  const statement_type statement_block::get_stmt_type() const {
    return statement_type::block_s;
  }

  std::vector<statement::ptr>& statement_block::get_contents() {
    return _contents;
  }

  bool statement_block::execute(runtime_context& context) const {
    for (statement::ptr stmt : _contents) {
      if (stmt && !stmt->execute(context))
        return false;
    }
    return true;
  }

  std::string statement_block::to_string() const {
    std::string str = "<BLOCK ";
    std::string delimiter = "";
    if (_contents.empty())
      str += "(empty)";
    for (statement::ptr stmt : _contents) {
      if (stmt) {
        str += delimiter + stmt->to_string();
        delimiter = ", ";
      }
    }
    return str + ">";
  }

  // --- statement_declare ---
  
  statement_declare::statement_declare(statement::ptr parent, const identifier_info* id_info, const std::string& id_name, statement::ptr value, size_t line_number, size_t char_index):
  _id_info(*id_info), _id_name(id_name), _value(value), statement(parent, line_number, char_index) {
  }

  const statement_type statement_declare::get_stmt_type() const {
    return statement_type::declare_s;
  }

  const identifier_info* statement_declare::get_id_info() const {
    return &_id_info;
  }

  const std::string& statement_declare::get_id_name() const {
    return _id_name;
  }

  statement::ptr statement_declare::get_value() const {
    return _value;
  }

  bool statement_declare::execute(runtime_context& context) const {
    return true;
  }

  std::string statement_declare::to_string() const {
    std::string str = "<DECLARE ";
    if (_id_info.is_final())
      str += "final ";
    if (_id_info.is_static())
      str += "static ";
    str += std::to_string(_id_info.type_id()) + " ";
    str += _id_name;
    if (_value)
      str += " = " + _value->to_string();
    return str + ">";
  }

  // --- statement_decfunc ---
  
  statement_decfunc::statement_decfunc(statement::ptr parent, const identifier_info* id_info, const std::string& id_name, std::vector<identifier_info> param_infos, std::vector<std::string> param_names, statement::ptr exec, size_t line_number, size_t char_index):
      _id_info(*id_info), _id_name(id_name), _param_infos(std::move(param_infos)), _param_names(std::move(param_names)), _exec(exec), statement(parent, line_number, char_index) {
  }

  const statement_type statement_decfunc::get_stmt_type() const {
    return statement_type::decfunc_s;
  }

  const identifier_info* statement_decfunc::get_id_info() const {
    return &_id_info;
  }

  const std::string& statement_decfunc::get_id_name() const {
    return _id_name;
  }

  const std::vector<identifier_info>& statement_decfunc::get_param_infos() const {
    return _param_infos;
  }

  const std::vector<std::string>& statement_decfunc::get_param_names() const {
    return _param_names;
  }

  statement::ptr statement_decfunc::get_exec() const {
    return _exec;
  }

  bool statement_decfunc::execute(runtime_context& context) const {
    return true;
  }

  void statement_decfunc::do_return(runtime_context& context) const {
  }

  std::string statement_decfunc::to_string() const {
    std::string str = "<FUNCTION ";
    if (_id_info.is_final())
      str += "final ";
    if (_id_info.is_static())
      str += "static ";
    str += std::to_string(_id_info.type_id()) + " ";
    str += _id_name + "(";
    bool do_comma = false;
    for (size_t i = 0; i < _param_infos.size(); ++i) {
      str += (do_comma ? ", " : "") + std::to_string(_param_infos.at(i).type_id()) + " " + _param_names.at(i);
      do_comma = true;
    }
    str += ") DOES " + _exec->to_string();
    return str + ">";
  }

  // --- statement_return ---

  statement_return::statement_return(statement::ptr parent, statement::ptr value, size_t line_number, size_t char_index):
  _value(value), statement(parent, line_number, char_index) {
  }

  const statement_type statement_return::get_stmt_type() const {
    return statement_type::return_s;
  }

  statement::ptr statement_return::get_value() const {
    return _value;
  }

  bool statement_return::execute(runtime_context& context) const {
    return true;
  }

  std::string statement_return::to_string() const {
    if (_value)
      return "<RETURN " + _value->to_string() + ">";
    else
      return "<RETURN>";
  }

  // --- statement_break ---

  statement_break::statement_break(statement::ptr parent, size_t line_number, size_t char_index):
  statement(parent, line_number, char_index) {
  }

  const statement_type statement_break::get_stmt_type() const {
    return statement_type::break_s;
  }

  bool statement_break::execute(runtime_context& context) const {
    return true;
  }

  std::string statement_break::to_string() const {
    return "<BREAK>";
  }

  // --- statement_continue ---

  statement_continue::statement_continue(statement::ptr parent, size_t line_number, size_t char_index):
  statement(parent, line_number, char_index) {
  }

  const statement_type statement_continue::get_stmt_type() const {
    return statement_type::continue_s;
  }

  bool statement_continue::execute(runtime_context& context) const {
    return true;
  }

  std::string statement_continue::to_string() const {
    return "<CONTINUE>";
  }

  // --- statement_if_else ---

  statement_if_else::statement_if_else(statement::ptr parent, statement::ptr condition, statement::ptr do_if, statement::ptr do_else, size_t line_number, size_t char_index):
  _condition(condition), _do_if(do_if), _do_else(do_else), statement(parent, line_number, char_index) {
  }

  const statement_type statement_if_else::get_stmt_type() const {
    return statement_type::ifelse_s;
  }

  statement::ptr statement_if_else::get_condition() const {
    return _condition;
  }

  statement::ptr statement_if_else::get_do_if() const {
    return _do_if;
  }

  statement::ptr statement_if_else::get_do_else() const {
    return _do_else;
  }

  bool statement_if_else::execute(runtime_context& context) const {
    return true;
  }

  std::string statement_if_else::to_string() const {
    if (!_condition || !_do_if)
      return "";
    std::string str = "<IF " + _condition->to_string() + " THEN " + _do_if->to_string();
    if (_do_else)
      str += " ELSE " + _do_else->to_string();
    return str + ">";
  }

  // --- statement_while ---

  statement_while::statement_while(statement::ptr parent, statement::ptr condition, statement::ptr looped, size_t line_number, size_t char_index):
  _condition(condition), _looped(looped), statement(parent, line_number, char_index) {
  }

  const statement_type statement_while::get_stmt_type() const {
    return statement_type::while_s;
  }

  statement::ptr statement_while::get_condition() const {
    return _condition;
  }

  statement::ptr statement_while::get_looped() const {
    return _looped;
  }

  bool statement_while::execute(runtime_context& context) const {
    return true;
  }

  void statement_while::do_break(runtime_context& context) const {
  }

  void statement_while::do_continue(runtime_context& context) const {
  }

  std::string statement_while::to_string() const {
    if (!_condition || !_looped)
      return "";
    return "<WHILE " + _condition->to_string() + " DO " + _looped->to_string() + ">";
  }

  // --- statement_do_while ---

  statement_do_while::statement_do_while(statement::ptr parent, statement::ptr condition, statement::ptr looped, size_t line_number, size_t char_index): statement_while(parent, condition, looped, line_number, char_index) {
  }

  const statement_type statement_do_while::get_stmt_type() const {
    return statement_type::dowhile_s;
  }

  bool statement_do_while::execute(runtime_context& context) const {
    return true;
  }

  void statement_do_while::do_break(runtime_context& context) const {
  }

  void statement_do_while::do_continue(runtime_context& context) const {
  }

  std::string statement_do_while::to_string() const {
    if (!_looped || !_condition)
      return "";
    return "<DO " + _looped->to_string() + " WHILE " + _condition->to_string() + ">";
  }

  // --- statement_for ---

  statement_for::statement_for(statement::ptr parent, statement::ptr first, statement::ptr condition, statement::ptr on_iter, statement::ptr looped, size_t line_number, size_t char_index):
  _first(first), _condition(condition), _on_iter(on_iter), _looped(looped), statement(parent, line_number, char_index) {
  }

  const statement_type statement_for::get_stmt_type() const {
    return statement_type::for_s;
  }

  statement::ptr statement_for::get_first() const {
    return _first;
  }

  statement::ptr statement_for::get_condition() const {
    return _condition;
  }

  statement::ptr statement_for::get_on_iter() const {
    return _on_iter;
  }

  statement::ptr statement_for::get_looped() const {
    return _looped;
  }

  bool statement_for::execute(runtime_context& context) const {
    return true;
  }

  void statement_for::do_break(runtime_context& context) const {
  }

  void statement_for::do_continue(runtime_context& context) const {
  }

  std::string statement_for::to_string() const {
    if (!_first || !_condition || !_on_iter || !_looped)
      return "";
    return "<FOR init" + _first->to_string() + " test" + _condition->to_string() + " update" + _on_iter->to_string() + " DO " + _looped->to_string() + ">";
  }

  // --- statement_foreach ---

  statement_foreach::statement_foreach(statement::ptr parent, statement::ptr declared, statement::ptr iter, statement::ptr looped, size_t line_number, size_t char_index):
  _declared(declared), _iter(iter), _looped(looped), statement(parent, line_number, char_index) {
  }

  const statement_type statement_foreach::get_stmt_type() const {
    return statement_type::foreach_s;
  }

  statement::ptr statement_foreach::get_declared() const {
    return _declared;
  }

  statement::ptr statement_foreach::get_iter() const {
    return _iter;
  }

  statement::ptr statement_foreach::get_looped() const {
    return _looped;
  }

  bool statement_foreach::execute(runtime_context& context) const {
    return true;
  }

  void statement_foreach::do_break(runtime_context& context) const {
  }

  void statement_foreach::do_continue(runtime_context& context) const {
  }

  std::string statement_foreach::to_string() const {
    if (!_declared || !_iter || !_looped)
      return "<FOR EACH (deallocated)>";
    return "<FOR EACH item" + _declared->to_string() + " IN iter" + _iter->to_string() + " DO " + _looped->to_string() + ">";
  }

  // --- statement_switch ---

  statement_switch::statement_switch(statement::ptr parent, statement::ptr tested, statement::ptr contents, size_t line_number, size_t char_index):
  _tested(tested), _contents(contents), statement(parent, line_number, char_index) {
  }

  const statement_type statement_switch::get_stmt_type() const {
    return statement_type::switch_s;
  }

  statement::ptr statement_switch::get_tested() const {
    return _tested;
  }

  statement::ptr statement_switch::get_contents() const {
    return _contents;
  }

  bool statement_switch::execute(runtime_context& context) const {
    return true;
  }

  void statement_switch::do_break(runtime_context& context) const {
  }

  void statement_switch::do_continue(runtime_context& context) const {
  }

  std::string statement_switch::to_string() const {
    return "";
  }

  // --- statement_switch_case ---

  statement_switch_case::statement_switch_case(statement::ptr parent, node_ptr& test, size_t line_number, size_t char_index):
  _test(test), statement(parent, line_number, char_index) {
  }

  const statement_type statement_switch_case::get_stmt_type() const {
    return statement_type::case_s;
  }

  node_ptr& statement_switch_case::get_test() const {
    return _test;
  }

  bool statement_switch_case::execute(runtime_context& context) const {
    return true;
  }

  std::string statement_switch_case::to_string() const {
    return "";
  }

  // --- statement_try_catch ---

  statement_try_catch::statement_try_catch(statement::ptr parent, statement::ptr to_try, std::vector<catcher_info>& catchers, statement::ptr do_after, size_t line_number, size_t char_index):
  _to_try(to_try), _catchers(std::move(catchers)), _do_after(do_after), statement(parent, line_number, char_index) {
  }

  const statement_type statement_try_catch::get_stmt_type() const {
    return statement_type::trycatch_s;
  }

  statement::ptr statement_try_catch::get_to_try() const {
    return _to_try;
  }

  const catcher_info* statement_try_catch::find_catcher(type_handle except_type) const {
    for (const catcher_info& catcher : _catchers) {
      if (catcher.except_type == except_type) {
        return &catcher;
      }
    }
    return nullptr;
  }

  statement::ptr statement_try_catch::get_do_after() const {
    return _do_after;
  }

  bool statement_try_catch::execute(runtime_context& context) const {
    return true;
  }

  std::string statement_try_catch::to_string() const {
    return "";
  }
}
