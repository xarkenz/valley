#include "statement.hpp"

#include "errors.hpp"


namespace valley {

  // --- Statement ---

  Statement::Statement(Statement::Ptr parent, size_t lineNumber, size_t charIndex):
    _parent(parent),
    _line_number(lineNumber),
    _char_index(charIndex)
  {
  }

  std::weak_ptr<const Statement> Statement::parent() const {
    return _parent;
  }

  size_t Statement::lineNumber() const {
    return _line_number;
  }

  size_t Statement::charIndex() const {
    return _char_index;
  }

  void Statement::setParent(Statement::Ptr parent) const {
    _parent = parent;
  }

  /*bool Statement::execute(RuntimeContext& context) const {
    // Proceed normally by default
    return true;
  }

  void Statement::doReturn(RuntimeContext& context) const {
    if (Statement::Ptr parent = _parent.lock())
      parent->doReturn(context);
    else
      throw RuntimeError("'return' called outside function definition.", _line_number, _char_index);
  }

  void Statement::doBreak(RuntimeContext& context) const {
    if (Statement::Ptr parent = _parent.lock())
      parent->doBreak(context);
    else
      throw RuntimeError("'break' called outside loop or switch.", _line_number, _char_index);
  }

  void Statement::doContinue(RuntimeContext& context) const {
    if (Statement::Ptr parent = _parent.lock())
      parent->doContinue(context);
    else
      throw RuntimeError("'continue' called outside loop or switch.", _line_number, _char_index);
  }*/

  // --- StatementEmpty ---

  StatementEmpty::StatementEmpty(Statement::Ptr parent, size_t lineNumber, size_t charIndex):
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementEmpty::type() const {
    return StatementType::EMPTY;
  }

  std::string StatementEmpty::toString() const {
    return "<EMPTY>";
  }

  // --- StatementExpr ---

  StatementExpr::StatementExpr(Statement::Ptr parent, Expression::Ptr& root, size_t lineNumber, size_t charIndex):
    _root(std::move(root)),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementExpr::type() const {
    return StatementType::EXPR;
  }

  const Expression::Ptr& StatementExpr::root() const {
    return _root;
  }

  /*bool StatementExpr::execute(RuntimeContext& context) const {
    return true;
  }*/

  std::string StatementExpr::toString() const {
    if (!_root)
      return "(deallocated)";
    return "<EXPR [" + expressionRepr(_root) + "]>";
  }

  // --- StatementBlock ---

  StatementBlock::StatementBlock(Statement::Ptr parent, std::vector<Statement::Ptr> contents, size_t lineNumber, size_t charIndex):
    _contents(std::move(contents)),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementBlock::type() const {
    return StatementType::BLOCK;
  }

  std::vector<Statement::Ptr>& StatementBlock::contents() {
    return _contents;
  }

  /*bool StatementBlock::execute(RuntimeContext& context) const {
    for (Statement::Ptr stmt : _contents) {
      if (stmt && !stmt->execute(context))
        return false;
    }
    return true;
  }*/

  std::string StatementBlock::toString() const {
    std::string str = "<BLOCK ";
    std::string delimiter = "";
    if (_contents.empty())
      str += "(empty)";
    for (Statement::Ptr stmt : _contents) {
      if (stmt) {
        str += delimiter + stmt->toString();
        delimiter = ", ";
      }
    }
    return str + ">";
  }

  // --- StatementDeclare ---
  
  StatementDeclare::StatementDeclare(Statement::Ptr parent, const IdentifierInfo* info, const std::string& name, Statement::Ptr value, size_t lineNumber, size_t charIndex):
    _info(*info),
    _name(name),
    _value(value),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementDeclare::type() const {
    return StatementType::DECLARE;
  }

  const IdentifierInfo* StatementDeclare::info() const {
    return &_info;
  }

  const std::string& StatementDeclare::name() const {
    return _name;
  }

  Statement::Ptr StatementDeclare::value() const {
    return _value;
  }

  /*bool StatementDeclare::execute(RuntimeContext& context) const {
    return true;
  }*/

  std::string StatementDeclare::toString() const {
    std::string str = "<DECLARE ";
    if (_info.isFinal())
      str += "final ";
    if (_info.isStatic())
      str += "static ";
    str += typeHandleRepr(_info.type()) + " ";
    str += _name;
    if (_value)
      str += " = " + _value->toString();
    return str + ">";
  }

  // --- StatementDecfunc ---
  
  StatementDecfunc::StatementDecfunc(Statement::Ptr parent, const IdentifierInfo* info, const std::string& name, std::vector<IdentifierInfo> paramInfos, std::vector<std::string> paramNames, Statement::Ptr exec, size_t lineNumber, size_t charIndex):
    _info(*info),
    _name(name),
    _param_infos(std::move(paramInfos)),
    _param_names(std::move(paramNames)),
    _exec(exec),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementDecfunc::type() const {
    return StatementType::DECFUNC;
  }

  const IdentifierInfo* StatementDecfunc::info() const {
    return &_info;
  }

  const std::string& StatementDecfunc::name() const {
    return _name;
  }

  const std::vector<IdentifierInfo>& StatementDecfunc::paramInfos() const {
    return _param_infos;
  }

  const std::vector<std::string>& StatementDecfunc::paramNames() const {
    return _param_names;
  }

  Statement::Ptr StatementDecfunc::exec() const {
    return _exec;
  }

  /*bool StatementDecfunc::execute(RuntimeContext& context) const {
    return true;
  }

  void StatementDecfunc::doReturn(RuntimeContext& context) const {
  }*/

  std::string StatementDecfunc::toString() const {
    std::string str = "<FUNCTION ";
    if (_info.isFinal())
      str += "final ";
    if (_info.isStatic())
      str += "static ";
    str += typeHandleRepr(_info.type()) + " ";
    str += _name + "(";
    bool doComma = false;
    for (size_t i = 0; i < _param_infos.size(); ++i) {
      str += (doComma ? ", " : "") + typeHandleRepr(_param_infos.at(i).type()) + " " + _param_names.at(i);
      doComma = true;
    }
    str += ") DOES " + _exec->toString();
    return str + ">";
  }

  // --- StatementReturn ---

  StatementReturn::StatementReturn(Statement::Ptr parent, Statement::Ptr value, size_t lineNumber, size_t charIndex):
    _value(value),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementReturn::type() const {
    return StatementType::RETURN;
  }

  Statement::Ptr StatementReturn::value() const {
    return _value;
  }

  /*bool StatementReturn::execute(RuntimeContext& context) const {
    return true;
  }*/

  std::string StatementReturn::toString() const {
    if (_value)
      return "<RETURN " + _value->toString() + ">";
    return "<RETURN>";
  }

  // --- StatementBreak ---

  StatementBreak::StatementBreak(Statement::Ptr parent, size_t lineNumber, size_t charIndex):
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementBreak::type() const {
    return StatementType::BREAK;
  }

  /*bool StatementBreak::execute(RuntimeContext& context) const {
    return true;
  }*/

  std::string StatementBreak::toString() const {
    return "<BREAK>";
  }

  // --- StatementContinue ---

  StatementContinue::StatementContinue(Statement::Ptr parent, size_t lineNumber, size_t charIndex):
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementContinue::type() const {
    return StatementType::CONTINUE;
  }

  /*bool StatementContinue::execute(RuntimeContext& context) const {
    return true;
  }*/

  std::string StatementContinue::toString() const {
    return "<CONTINUE>";
  }

  // --- StatementIfElse ---

  StatementIfElse::StatementIfElse(Statement::Ptr parent, Statement::Ptr condition, Statement::Ptr doIf, Statement::Ptr doElse, size_t lineNumber, size_t charIndex):
    _condition(condition),
    _do_if(doIf),
    _do_else(doElse),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementIfElse::type() const {
    return StatementType::IF_ELSE;
  }

  Statement::Ptr StatementIfElse::condition() const {
    return _condition;
  }

  Statement::Ptr StatementIfElse::doIf() const {
    return _do_if;
  }

  Statement::Ptr StatementIfElse::doElse() const {
    return _do_else;
  }

  /*bool StatementIfElse::execute(RuntimeContext& context) const {
    return true;
  }*/

  std::string StatementIfElse::toString() const {
    if (!_condition || !_do_if)
      return "";
    std::string str = "<IF " + _condition->toString() + " THEN " + _do_if->toString();
    if (_do_else)
      str += " ELSE " + _do_else->toString();
    return str + ">";
  }

  // --- StatementWhile ---

  StatementWhile::StatementWhile(Statement::Ptr parent, Statement::Ptr condition, Statement::Ptr looped, size_t lineNumber, size_t charIndex):
    _condition(condition),
    _looped(looped),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementWhile::type() const {
    return StatementType::WHILE;
  }

  Statement::Ptr StatementWhile::condition() const {
    return _condition;
  }

  Statement::Ptr StatementWhile::looped() const {
    return _looped;
  }

  /*bool StatementWhile::execute(RuntimeContext& context) const {
    return true;
  }

  void StatementWhile::doBreak(RuntimeContext& context) const {
  }

  void StatementWhile::doContinue(RuntimeContext& context) const {
  }*/

  std::string StatementWhile::toString() const {
    if (!_condition || !_looped)
      return "";
    return "<WHILE " + _condition->toString() + " DO " + _looped->toString() + ">";
  }

  // --- StatementDoWhile ---

  StatementDoWhile::StatementDoWhile(Statement::Ptr parent, Statement::Ptr condition, Statement::Ptr looped, size_t lineNumber, size_t charIndex):
    StatementWhile(parent, condition, looped, lineNumber, charIndex)
  {
  }

  const StatementType StatementDoWhile::type() const {
    return StatementType::DO_WHILE;
  }

  /*bool StatementDoWhile::execute(RuntimeContext& context) const {
    return true;
  }

  void StatementDoWhile::doBreak(RuntimeContext& context) const {
  }

  void StatementDoWhile::doContinue(RuntimeContext& context) const {
  }*/

  std::string StatementDoWhile::toString() const {
    if (!_looped || !_condition)
      return "";
    return "<DO " + _looped->toString() + " WHILE " + _condition->toString() + ">";
  }

  // --- StatementFor ---

  StatementFor::StatementFor(Statement::Ptr parent, Statement::Ptr first, Statement::Ptr condition, Statement::Ptr onIter, Statement::Ptr looped, size_t lineNumber, size_t charIndex):
    _first(first),
    _condition(condition),
    _on_iter(onIter),
    _looped(looped),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementFor::type() const {
    return StatementType::FOR;
  }

  Statement::Ptr StatementFor::first() const {
    return _first;
  }

  Statement::Ptr StatementFor::condition() const {
    return _condition;
  }

  Statement::Ptr StatementFor::onIter() const {
    return _on_iter;
  }

  Statement::Ptr StatementFor::looped() const {
    return _looped;
  }

  /*bool StatementFor::execute(RuntimeContext& context) const {
    return true;
  }

  void StatementFor::doBreak(RuntimeContext& context) const {
  }

  void StatementFor::doContinue(RuntimeContext& context) const {
  }*/

  std::string StatementFor::toString() const {
    if (!_first || !_condition || !_on_iter || !_looped)
      return "";
    return "<FOR init" + _first->toString() + " test" + _condition->toString() + " update" + _on_iter->toString() + " DO " + _looped->toString() + ">";
  }

  // --- StatementForeach ---

  StatementForeach::StatementForeach(Statement::Ptr parent, Statement::Ptr declared, Statement::Ptr iter, Statement::Ptr looped, size_t lineNumber, size_t charIndex):
    _declared(declared),
    _iter(iter),
    _looped(looped),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementForeach::type() const {
    return StatementType::FOREACH;
  }

  Statement::Ptr StatementForeach::declared() const {
    return _declared;
  }

  Statement::Ptr StatementForeach::iter() const {
    return _iter;
  }

  Statement::Ptr StatementForeach::looped() const {
    return _looped;
  }

  /*bool StatementForeach::execute(RuntimeContext& context) const {
    return true;
  }

  void StatementForeach::doBreak(RuntimeContext& context) const {
  }

  void StatementForeach::doContinue(RuntimeContext& context) const {
  }*/

  std::string StatementForeach::toString() const {
    if (!_declared || !_iter || !_looped)
      return "<FOREACH (deallocated)>";
    return "<FOREACH item" + _declared->toString() + " IN iter" + _iter->toString() + " DO " + _looped->toString() + ">";
  }

  // --- StatementSwitch ---

  StatementSwitch::StatementSwitch(Statement::Ptr parent, Statement::Ptr tested, Statement::Ptr contents, size_t lineNumber, size_t charIndex):
    _tested(tested),
    _contents(contents),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementSwitch::type() const {
    return StatementType::SWITCH;
  }

  Statement::Ptr StatementSwitch::tested() const {
    return _tested;
  }

  Statement::Ptr StatementSwitch::contents() const {
    return _contents;
  }

  /*bool StatementSwitch::execute(RuntimeContext& context) const {
    return true;
  }

  void StatementSwitch::doBreak(RuntimeContext& context) const {
  }

  void StatementSwitch::doContinue(RuntimeContext& context) const {
  }*/

  std::string StatementSwitch::toString() const {
    return "";
  }

  // --- StatementSwitchCase ---

  StatementSwitchCase::StatementSwitchCase(Statement::Ptr parent, Expression::Ptr& test, size_t lineNumber, size_t charIndex):
    _test(std::move(test)),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementSwitchCase::type() const {
    return StatementType::CASE;
  }

  const Expression::Ptr& StatementSwitchCase::test() const {
    return _test;
  }

  /*bool StatementSwitchCase::execute(RuntimeContext& context) const {
    return true;
  }*/

  std::string StatementSwitchCase::toString() const {
    return "";
  }

  // --- StatementTryCatch ---

  StatementTryCatch::StatementTryCatch(Statement::Ptr parent, Statement::Ptr toTry, std::vector<ExceptionCatcherInfo>& catchers, Statement::Ptr doAfter, size_t lineNumber, size_t charIndex):
    _to_try(toTry),
    _catchers(std::move(catchers)),
    _do_after(doAfter),
    Statement(parent, lineNumber, charIndex)
  {
  }

  const StatementType StatementTryCatch::type() const {
    return StatementType::TRY_CATCH;
  }

  Statement::Ptr StatementTryCatch::toTry() const {
    return _to_try;
  }

  const ExceptionCatcherInfo* StatementTryCatch::findCatcher(TypeHandle exceptType) const {
    for (const ExceptionCatcherInfo& catcher : _catchers) {
      if (catcher.type == exceptType) {
        return &catcher;
      }
    }
    return nullptr;
  }

  Statement::Ptr StatementTryCatch::doAfter() const {
    return _do_after;
  }

  /*bool StatementTryCatch::execute(RuntimeContext& context) const {
    return true;
  }*/

  std::string StatementTryCatch::toString() const {
    return "";
  }

}