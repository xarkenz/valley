#pragma once

#include "compiler_context.hpp"
#include "expression_manager.hpp"
//#include "runtime_context.hpp"


namespace valley {

  enum struct StatementType {
    EMPTY,
    EXPR,
    BLOCK,
    DECLARE,
    DECFUNC,
    RETURN,
    BREAK,
    CONTINUE,
    IF_ELSE,
    WHILE,
    DO_WHILE,
    FOR,
    FOREACH,
    SWITCH,
    CASE,
    TRY_CATCH,
  };
  
  class Statement: std::enable_shared_from_this<Statement> {
  private:
    Statement(const Statement&) = delete;
    void operator=(const Statement&) = delete;
  
  public:
    using Ptr = std::shared_ptr<const Statement>;

    virtual ~Statement() = default;
    virtual const StatementType type() const = 0;

    std::weak_ptr<const Statement> parent() const;
    size_t lineNumber() const;
    size_t charIndex() const;
    void setParent(Statement::Ptr parent) const;

    //bool execute(RuntimeContext& context) const;
    //void doReturn(RuntimeContext& context) const;
    //void doBreak(RuntimeContext& context) const;
    //void doContinue(RuntimeContext& context) const;

    virtual std::string toString() const = 0;
  
  protected:
    Statement(Statement::Ptr parent, size_t lineNumber, size_t charIndex);

    mutable std::weak_ptr<const Statement> _parent;
    size_t _line_number;
    size_t _char_index;
  };

  // Empty statement
  class StatementEmpty: public Statement {
  public:
    StatementEmpty(Statement::Ptr parent, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    std::string toString() const;
  };

  // <root: <expr>>
  class StatementExpr: public Statement {
  private:
    Expression::Ptr _root;

  public:
    StatementExpr(Statement::Ptr parent, Expression::Ptr& root, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    const Expression::Ptr& root() const;

    //bool execute(RuntimeContext& context) const;

    std::string toString() const;
  };

  // {<contents...>}
  class StatementBlock: public Statement {
  private:
    std::vector<Statement::Ptr> _contents;
  
  public:
    StatementBlock(Statement::Ptr parent, std::vector<Statement::Ptr> contents, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    std::vector<Statement::Ptr>& contents();

    //bool execute(RuntimeContext& context) const;

    std::string toString() const;
  };

  // <info> <name> [= <value>]
  class StatementDeclare: public Statement {
  private:
    const IdentifierInfo _info;
    const std::string _name;
    Statement::Ptr _value;
  
  public:
    StatementDeclare(Statement::Ptr parent, const IdentifierInfo* info, const std::string& name, Statement::Ptr value, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    const IdentifierInfo* info() const;
    const std::string& name() const;
    Statement::Ptr value() const;

    //bool execute(RuntimeContext& context) const;

    std::string toString() const;
  };

  // <info> <name>(<info> <name>, ...) [<exec>]
  class StatementDecfunc: public Statement {
  private:
    const IdentifierInfo _info;
    const std::string _name;
    std::vector<IdentifierInfo> _param_infos;
    std::vector<std::string> _param_names;
    Statement::Ptr _exec;
  
  public:
    StatementDecfunc(Statement::Ptr parent, const IdentifierInfo* info, const std::string& name, std::vector<IdentifierInfo> paramInfos, std::vector<std::string> paramNames, Statement::Ptr exec, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    const IdentifierInfo* info() const;
    const std::string& name() const;
    const std::vector<IdentifierInfo>& paramInfos() const;
    const std::vector<std::string>& paramNames() const;
    Statement::Ptr exec() const;

    //bool execute(RuntimeContext& context) const;
    //void doReturn(RuntimeContext& context) const;
    
    std::string toString() const;
  };

  // return [<value>]
  class StatementReturn: public Statement {
  private:
    Statement::Ptr _value;
  
  public:
    StatementReturn(Statement::Ptr parent, Statement::Ptr value, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    Statement::Ptr value() const;

    //bool execute(RuntimeContext& context) const;

    std::string toString() const;
  };

  // break
  class StatementBreak: public Statement {
  public:
    StatementBreak(Statement::Ptr parent, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    //bool execute(RuntimeContext& context) const;

    std::string toString() const;
  };

  // continue
  class StatementContinue: public Statement {
  public:
    StatementContinue(Statement::Ptr parent, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    //bool execute(RuntimeContext& context) const;

    std::string toString() const;
  };

  // if (<condition>) <doIf> [else <doElse>]
  class StatementIfElse: public Statement {
  private:
    Statement::Ptr _condition;
    Statement::Ptr _do_if;
    Statement::Ptr _do_else;
  
  public:
    StatementIfElse(Statement::Ptr parent, Statement::Ptr condition, Statement::Ptr doIf, Statement::Ptr doElse, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    Statement::Ptr condition() const;
    Statement::Ptr doIf() const;
    Statement::Ptr doElse() const;

    //bool execute(RuntimeContext& context) const;

    std::string toString() const;
  };

  // while (<condition>) <looped>
  class StatementWhile: public Statement {
  protected:
    Statement::Ptr _condition;
    Statement::Ptr _looped;
  
  public:
    StatementWhile(Statement::Ptr parent, Statement::Ptr condition, Statement::Ptr looped, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    Statement::Ptr condition() const;
    Statement::Ptr looped() const;

    //bool execute(RuntimeContext& context) const;
    //void doBreak(RuntimeContext& context) const;
    //void doContinue(RuntimeContext& context) const;

    std::string toString() const;
  };

  // do <looped> while (<condition>);
  class StatementDoWhile: public StatementWhile {
  public:
    StatementDoWhile(Statement::Ptr parent, Statement::Ptr condition, Statement::Ptr looped, size_t lineNumber, size_t charIndex);
    const StatementType type() const override;

    //bool execute(RuntimeContext& context) const;
    //void doBreak(RuntimeContext& context) const;
    //void doContinue(RuntimeContext& context) const;
    
    std::string toString() const override;
  };

  // for ([<first>]; [<condition>]; [<onIter>]) <looped>
  class StatementFor: public Statement {
  private:
    Statement::Ptr _first;
    Statement::Ptr _condition;
    Statement::Ptr _on_iter;
    Statement::Ptr _looped;
  
  public:
    StatementFor(Statement::Ptr parent, Statement::Ptr first, Statement::Ptr condition, Statement::Ptr onIter, Statement::Ptr looped, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    Statement::Ptr first() const;
    Statement::Ptr condition() const;
    Statement::Ptr onIter() const;
    Statement::Ptr looped() const;

    //bool execute(RuntimeContext& context) const;
    //void doBreak(RuntimeContext& context) const;
    //void doContinue(RuntimeContext& context) const;

    std::string toString() const;
  };

  // for (<declared> : <iter>) <looped>
  class StatementForeach: public Statement {
  private:
    Statement::Ptr _declared;
    Statement::Ptr _iter;
    Statement::Ptr _looped;
  
  public:
    StatementForeach(Statement::Ptr parent, Statement::Ptr declared, Statement::Ptr iter, Statement::Ptr looped, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    Statement::Ptr declared() const;
    Statement::Ptr iter() const;
    Statement::Ptr looped() const;

    //bool execute(RuntimeContext& context) const;
    //void doBreak(RuntimeContext& context) const;
    //void doContinue(RuntimeContext& context) const;

    std::string toString() const;
  };

  // switch (<tested>) <contents>
  class StatementSwitch: public Statement {
  private:
    Statement::Ptr _tested;
    Statement::Ptr _contents;
  
  public:
    StatementSwitch(Statement::Ptr parent, Statement::Ptr tested, Statement::Ptr contents, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    Statement::Ptr tested() const;
    Statement::Ptr contents() const;

    //bool execute(RuntimeContext& context) const;
    //void doBreak(RuntimeContext& context) const;
    //void doContinue(RuntimeContext& context) const;

    std::string toString() const;
  };

  // case [<test>]: | default:
  class StatementSwitchCase: public Statement {
  private:
    Expression::Ptr _test;
  
  public:
    StatementSwitchCase(Statement::Ptr parent, Expression::Ptr& test, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    const Expression::Ptr& test() const;

    //bool execute(RuntimeContext& context) const;

    std::string toString() const;
  };

  // (catch [(<type> [<name>])] [<onCatch>])
  struct ExceptionCatcherInfo {
    TypeHandle type;
    const std::string name;
    Statement::Ptr onCatch;
  };
  
  // try <toTry> [catch (<type> <name>) <onCatch> [catch ...]] [finally <doAfter>]
  class StatementTryCatch: public Statement {
  private:
    Statement::Ptr _to_try;
    std::vector<ExceptionCatcherInfo> _catchers;
    Statement::Ptr _do_after;
  
  public:
    StatementTryCatch(Statement::Ptr parent, Statement::Ptr toTry, std::vector<ExceptionCatcherInfo>& catchers, Statement::Ptr doAfter, size_t lineNumber, size_t charIndex);
    const StatementType type() const;

    Statement::Ptr toTry() const;
    const ExceptionCatcherInfo* findCatcher(TypeHandle exceptType) const;
    Statement::Ptr doAfter() const;

    //bool execute(RuntimeContext& context) const;

    std::string toString() const;
  };

}