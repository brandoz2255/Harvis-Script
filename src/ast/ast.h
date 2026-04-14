#ifndef HARVIS_AST_AST_H
#define HARVIS_AST_AST_H

#include "../core/types.h"
#include "../lexer/token.h"
#include <vector>
#include <memory>
#include <variant>
#include <string>

namespace hs {

// ============ FORWARD DECLARATIONS ============

class Visitor;
class StatementVisitor;

class Expr;
class Stmt;

// Expression types
class LiteralExpr;
class IdentifierExpr;
class BinaryExpr;
class UnaryExpr;
class GroupingExpr;
class CallExpr;
class ArrayLiteralExpr;
class ObjectLiteralExpr;
class IndexExpr;
class DotExpr;
class AssignExpr;
class LogicalExpr;
class TernaryExpr;
class LambdaExpr;
class NewExpr;
class ThisExpr;
class SuperExpr;
class TypeCastExpr;

// Statement types
class ExpressionStmt;
class BlockStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class DoWhileStmt;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;
class FunctionStmt;
class ClassStmt;
class VarDeclStmt;
class ConstDeclStmt;
class ImportStmt;
class ExportStmt;
class TryStmt;
class ThrowStmt;
class SwitchStmt;

// ============ OPERATOR ENUMS ============

enum class BinaryOp {
    EQUAL,  // Assignment =
    PLUS, MINUS, STAR, SLASH, PERCENT,
    PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL,
    EQUAL_EQUAL, BANG_EQUAL,
    LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    AND, OR,
    AMPERSAND, PIPE, CARET, TILDE,
    LESS_LESS, GREATER_GREATER,
    INSTANCE_OF, IN, TYPEOF
};

enum class UnaryOp {
    MINUS, BANG, TILDE, PLUS_PLUS, MINUS_MINUS, TYPEOF, VOID_OP, DELETE_OP
};

// ============ BASE NODES ============

class AstNode {
public:
    SourceLocation location;
    
    AstNode(SourceLocation loc) : location(loc) {}
    virtual ~AstNode() = default;
    
    virtual std::string toString() const = 0;
};

class Expr : public AstNode {
public:
    using Ptr = std::shared_ptr<Expr>;
    
    Expr(SourceLocation loc) : AstNode(loc) {}
    virtual ~Expr() = default;
    
    virtual void accept(Visitor& visitor) = 0;
};

class Stmt : public AstNode {
public:
    using Ptr = std::shared_ptr<Stmt>;
    
    Stmt(SourceLocation loc) : AstNode(loc) {}
    virtual ~Stmt() = default;
    
    virtual void accept(StatementVisitor& visitor) = 0;
};

// ============ VISITOR INTERFACES ============

class Visitor {
public:
    virtual ~Visitor() = default;
    
    virtual void visitLiteralExpr(LiteralExpr* expr) = 0;
    virtual void visitIdentifierExpr(IdentifierExpr* expr) = 0;
    virtual void visitBinaryExpr(BinaryExpr* expr) = 0;
    virtual void visitUnaryExpr(UnaryExpr* expr) = 0;
    virtual void visitGroupingExpr(GroupingExpr* expr) = 0;
    virtual void visitCallExpr(CallExpr* expr) = 0;
    virtual void visitArrayLiteralExpr(ArrayLiteralExpr* expr) = 0;
    virtual void visitObjectLiteralExpr(ObjectLiteralExpr* expr) = 0;
    virtual void visitIndexExpr(IndexExpr* expr) = 0;
    virtual void visitDotExpr(DotExpr* expr) = 0;
    virtual void visitAssignExpr(AssignExpr* expr) = 0;
    virtual void visitLogicalExpr(LogicalExpr* expr) = 0;
    virtual void visitTernaryExpr(TernaryExpr* expr) = 0;
    virtual void visitLambdaExpr(LambdaExpr* expr) = 0;
    virtual void visitNewExpr(NewExpr* expr) = 0;
    virtual void visitThisExpr(ThisExpr* expr) = 0;
    virtual void visitSuperExpr(SuperExpr* expr) = 0;
    virtual void visitTypeCastExpr(TypeCastExpr* expr) = 0;
};

class StatementVisitor {
public:
    virtual ~StatementVisitor() = default;
    
    virtual void visitExpressionStmt(ExpressionStmt* stmt) = 0;
    virtual void visitBlockStmt(BlockStmt* stmt) = 0;
    virtual void visitIfStmt(IfStmt* stmt) = 0;
    virtual void visitWhileStmt(WhileStmt* stmt) = 0;
    virtual void visitForStmt(ForStmt* stmt) = 0;
    virtual void visitDoWhileStmt(DoWhileStmt* stmt) = 0;
    virtual void visitReturnStmt(ReturnStmt* stmt) = 0;
    virtual void visitBreakStmt(BreakStmt* stmt) = 0;
    virtual void visitContinueStmt(ContinueStmt* stmt) = 0;
    virtual void visitFunctionStmt(FunctionStmt* stmt) = 0;
    virtual void visitClassStmt(ClassStmt* stmt) = 0;
    virtual void visitVarDeclStmt(VarDeclStmt* stmt) = 0;
    virtual void visitConstDeclStmt(ConstDeclStmt* stmt) = 0;
    virtual void visitImportStmt(ImportStmt* stmt) = 0;
    virtual void visitExportStmt(ExportStmt* stmt) = 0;
    virtual void visitTryStmt(TryStmt* stmt) = 0;
    virtual void visitThrowStmt(ThrowStmt* stmt) = 0;
    virtual void visitSwitchStmt(SwitchStmt* stmt) = 0;
};

// ============ LITERAL EXPRESSIONS ============

class LiteralExpr : public Expr {
public:
    Value value;
    Type type;
    
    LiteralExpr(SourceLocation loc, Value val, Type t = Type::undefined())
        : Expr(loc), value(std::move(val)), type(t) {}
    
    void accept(Visitor& visitor) override { visitor.visitLiteralExpr(this); }
    std::string toString() const override {
        return "LiteralExpr(" + value.toString() + ")";
    }
};

class IdentifierExpr : public Expr {
public:
    std::string name;
    Type declaredType;
    
    IdentifierExpr(SourceLocation loc, std::string n, Type t = Type::undefined())
        : Expr(loc), name(std::move(n)), declaredType(t) {}
    
    void accept(Visitor& visitor) override { visitor.visitIdentifierExpr(this); }
    std::string toString() const override {
        return "IdentifierExpr(" + name + ")";
    }
};

class ThisExpr : public Expr {
public:
    ThisExpr(SourceLocation loc) : Expr(loc) {}
    void accept(Visitor& visitor) override { visitor.visitThisExpr(this); }
    std::string toString() const override { return "ThisExpr"; }
};

class SuperExpr : public Expr {
public:
    std::string methodName;
    std::vector<Expr::Ptr> arguments;
    
    SuperExpr(SourceLocation loc, std::string method, std::vector<Expr::Ptr> args)
        : Expr(loc), methodName(std::move(method)), arguments(std::move(args)) {}
    
    void accept(Visitor& visitor) override { visitor.visitSuperExpr(this); }
    std::string toString() const override { return "SuperExpr(" + methodName + ")"; }
};

// ============ OPERATOR EXPRESSIONS ============

class BinaryExpr : public Expr {
public:
    Expr::Ptr left;
    BinaryOp op;
    Expr::Ptr right;
    
    BinaryExpr(SourceLocation loc, Expr::Ptr l, BinaryOp o, Expr::Ptr r)
        : Expr(loc), left(std::move(l)), op(o), right(std::move(r)) {}
    
    void accept(Visitor& visitor) override { visitor.visitBinaryExpr(this); }
    std::string toString() const override {
        return "BinaryExpr(" + left->toString() + " " + std::to_string(static_cast<int>(op)) + " " + right->toString() + ")";
    }
    
    static std::string opToString(BinaryOp op);
};

class UnaryExpr : public Expr {
public:
    UnaryOp op;
    Expr::Ptr operand;
    bool prefix;
    
    UnaryExpr(SourceLocation loc, UnaryOp o, Expr::Ptr op, bool pre = true)
        : Expr(loc), op(o), operand(std::move(op)), prefix(pre) {}
    
    void accept(Visitor& visitor) override { visitor.visitUnaryExpr(this); }
    std::string toString() const override {
        std::string prefixStr = prefix ? "" : "Post";
        return prefixStr + "UnaryExpr(" + opToString(op) + " " + operand->toString() + ")";
    }
    
    static std::string opToString(UnaryOp op);
};

class GroupingExpr : public Expr {
public:
    Expr::Ptr expression;
    
    GroupingExpr(SourceLocation loc, Expr::Ptr expr)
        : Expr(loc), expression(std::move(expr)) {}
    
    void accept(Visitor& visitor) override { visitor.visitGroupingExpr(this); }
    std::string toString() const override {
        return "GroupingExpr(" + expression->toString() + ")";
    }
};

// ============ COMPLEX EXPRESSIONS ============

class CallExpr : public Expr {
public:
    Expr::Ptr callee;
    Token paren;
    std::vector<Expr::Ptr> arguments;
    std::string typeParams;
    
    CallExpr(SourceLocation loc, Expr::Ptr c, Token p, std::vector<Expr::Ptr> args, std::string tp = "")
        : Expr(loc), callee(std::move(c)), paren(std::move(p)), 
          arguments(std::move(args)), typeParams(std::move(tp)) {}
    
    void accept(Visitor& visitor) override { visitor.visitCallExpr(this); }
    std::string toString() const override {
        return "CallExpr(" + callee->toString() + "...";
    }
};

class ArrayLiteralExpr : public Expr {
public:
    std::vector<Expr::Ptr> elements;
    Type elementType;
    
    ArrayLiteralExpr(SourceLocation loc, std::vector<Expr::Ptr> elems, Type et = Type::undefined())
        : Expr(loc), elements(std::move(elems)), elementType(et) {}
    
    void accept(Visitor& visitor) override { visitor.visitArrayLiteralExpr(this); }
    std::string toString() const override { return "ArrayLiteralExpr([...])"; }
};

class ObjectLiteralExpr : public Expr {
public:
    struct Property {
        std::string name;
        Expr::Ptr value;
        bool isMethod;
        bool isGetter;
        bool isSetter;
        bool isSpread;
        Type paramType;
    };
    
    std::vector<Property> properties;
    
    ObjectLiteralExpr(SourceLocation loc, std::vector<Property> props)
        : Expr(loc), properties(std::move(props)) {}
    
    void accept(Visitor& visitor) override { visitor.visitObjectLiteralExpr(this); }
    std::string toString() const override { return "ObjectLiteralExpr({...})"; }
};

class IndexExpr : public Expr {
public:
    Expr::Ptr object;
    Expr::Ptr index;
    bool optional;
    
    IndexExpr(SourceLocation loc, Expr::Ptr obj, Expr::Ptr idx, bool opt = false)
        : Expr(loc), object(std::move(obj)), index(std::move(idx)), optional(opt) {}
    
    void accept(Visitor& visitor) override { visitor.visitIndexExpr(this); }
    std::string toString() const override {
        return "IndexExpr(" + object->toString() + "[...]";
    }
};

class DotExpr : public Expr {
public:
    Expr::Ptr object;
    std::string property;
    bool optional;
    Type propertyType;
    
    DotExpr(SourceLocation loc, Expr::Ptr obj, std::string prop, bool opt = false, Type pt = Type::undefined())
        : Expr(loc), object(std::move(obj)), property(std::move(prop)), 
          optional(opt), propertyType(pt) {}
    
    void accept(Visitor& visitor) override { visitor.visitDotExpr(this); }
    std::string toString() const override {
        return "DotExpr(" + object->toString() + "." + property + ")";
    }
};

class AssignExpr : public Expr {
public:
    Expr::Ptr target;
    BinaryOp op;
    Expr::Ptr value;
    
    AssignExpr(SourceLocation loc, Expr::Ptr t, BinaryOp o, Expr::Ptr v)
        : Expr(loc), target(std::move(t)), op(o), value(std::move(v)) {}
    
    void accept(Visitor& visitor) override { visitor.visitAssignExpr(this); }
    std::string toString() const override {
        return "AssignExpr(" + target->toString() + " " + BinaryExpr::opToString(op) + " ...)";
    }
};

class LogicalExpr : public Expr {
public:
    Expr::Ptr left;
    bool isAnd;
    Expr::Ptr right;
    
    LogicalExpr(SourceLocation loc, Expr::Ptr l, bool andOp, Expr::Ptr r)
        : Expr(loc), left(std::move(l)), isAnd(andOp), right(std::move(r)) {}
    
    void accept(Visitor& visitor) override { visitor.visitLogicalExpr(this); }
    std::string toString() const override {
        return "LogicalExpr(" + left->toString() + " " + (isAnd ? "&&" : "||") + " ...)";
    }
};

class TernaryExpr : public Expr {
public:
    Expr::Ptr condition;
    Expr::Ptr thenExpr;
    Expr::Ptr elseExpr;
    
    TernaryExpr(SourceLocation loc, Expr::Ptr cond, Expr::Ptr th, Expr::Ptr el)
        : Expr(loc), condition(std::move(cond)), thenExpr(std::move(th)), elseExpr(std::move(el)) {}
    
    void accept(Visitor& visitor) override { visitor.visitTernaryExpr(this); }
    std::string toString() const override { return "TernaryExpr(... ? ... : ...)"; }
};

class LambdaExpr : public Expr {
public:
    std::vector<std::pair<std::string, Type>> parameters;
    std::string bodyType;
    Expr::Ptr body;
    std::vector<Stmt::Ptr> block;
    bool isAsync;
    bool isGenerator;
    std::string name;
    
    LambdaExpr(SourceLocation loc, std::vector<std::pair<std::string, Type>> params,
               std::string bt, Expr::Ptr b, std::vector<Stmt::Ptr> blk,
               bool async = false, bool gen = false, std::string n = "")
        : Expr(loc), parameters(std::move(params)), bodyType(std::move(bt)),
          body(std::move(b)), block(std::move(blk)), isAsync(async),
          isGenerator(gen), name(std::move(n)) {}
    
    void accept(Visitor& visitor) override { visitor.visitLambdaExpr(this); }
    std::string toString() const override { return "LambdaExpr((...) => ...)"; }
};

class NewExpr : public Expr {
public:
    Expr::Ptr callee;
    std::vector<Expr::Ptr> arguments;
    
    NewExpr(SourceLocation loc, Expr::Ptr c, std::vector<Expr::Ptr> args)
        : Expr(loc), callee(std::move(c)), arguments(std::move(args)) {}
    
    void accept(Visitor& visitor) override { visitor.visitNewExpr(this); }
    std::string toString() const override { return "NewExpr(" + callee->toString() + ")"; }
};

class TypeCastExpr : public Expr {
public:
    Expr::Ptr expression;
    Type targetType;
    
    TypeCastExpr(SourceLocation loc, Expr::Ptr expr, Type target)
        : Expr(loc), expression(std::move(expr)), targetType(target) {}
    
    void accept(Visitor& visitor) override { visitor.visitTypeCastExpr(this); }
    std::string toString() const override { return "TypeCastExpr(... as ...)"; }
};

// ============ STATEMENTS ============

class ExpressionStmt : public Stmt {
public:
    Expr::Ptr expression;
    
    ExpressionStmt(SourceLocation loc, Expr::Ptr expr)
        : Stmt(loc), expression(std::move(expr)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitExpressionStmt(this); }
    std::string toString() const override { return "ExpressionStmt(" + expression->toString() + ")"; }
};

class BlockStmt : public Stmt {
public:
    std::vector<Stmt::Ptr> statements;
    
    BlockStmt(SourceLocation loc, std::vector<Stmt::Ptr> stmts)
        : Stmt(loc), statements(std::move(stmts)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitBlockStmt(this); }
    std::string toString() const override { return "BlockStmt({...})"; }
};

class IfStmt : public Stmt {
public:
    Expr::Ptr condition;
    Stmt::Ptr thenBranch;
    Stmt::Ptr elseBranch;
    
    IfStmt(SourceLocation loc, Expr::Ptr cond, Stmt::Ptr th, Stmt::Ptr el = nullptr)
        : Stmt(loc), condition(std::move(cond)), thenBranch(std::move(th)), elseBranch(std::move(el)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitIfStmt(this); }
    std::string toString() const override { return "IfStmt(... ? ... : ...)"; }
};

class WhileStmt : public Stmt {
public:
    Expr::Ptr condition;
    Stmt::Ptr body;
    
    WhileStmt(SourceLocation loc, Expr::Ptr cond, Stmt::Ptr body)
        : Stmt(loc), condition(std::move(cond)), body(std::move(body)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitWhileStmt(this); }
    std::string toString() const override { return "WhileStmt(... {...})"; }
};

class ForStmt : public Stmt {
public:
    std::variant<Expr::Ptr, std::shared_ptr<VarDeclStmt>> init;
    Expr::Ptr condition;
    Expr::Ptr increment;
    Stmt::Ptr body;
    
    ForStmt(SourceLocation loc, std::variant<Expr::Ptr, std::shared_ptr<VarDeclStmt>> i,
            Expr::Ptr cond, Expr::Ptr inc, Stmt::Ptr b)
        : Stmt(loc), init(std::move(i)), condition(std::move(cond)),
          increment(std::move(inc)), body(std::move(b)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitForStmt(this); }
    std::string toString() const override { return "ForStmt(... {...})"; }
};

class DoWhileStmt : public Stmt {
public:
    Stmt::Ptr body;
    Expr::Ptr condition;
    
    DoWhileStmt(SourceLocation loc, Stmt::Ptr b, Expr::Ptr cond)
        : Stmt(loc), body(std::move(b)), condition(std::move(cond)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitDoWhileStmt(this); }
    std::string toString() const override { return "DoWhileStmt({...} ...)"; }
};

class ReturnStmt : public Stmt {
public:
    Expr::Ptr value;
    
    ReturnStmt(SourceLocation loc, Expr::Ptr v = nullptr)
        : Stmt(loc), value(std::move(v)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitReturnStmt(this); }
    std::string toString() const override { 
        return value ? "ReturnStmt(...)" : "ReturnStmt();"; 
    }
};

class BreakStmt : public Stmt {
public:
    std::string label;
    
    BreakStmt(SourceLocation loc, std::string lbl = "")
        : Stmt(loc), label(std::move(lbl)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitBreakStmt(this); }
    std::string toString() const override { return "BreakStmt"; }
};

class ContinueStmt : public Stmt {
public:
    std::string label;
    
    ContinueStmt(SourceLocation loc, std::string lbl = "")
        : Stmt(loc), label(std::move(lbl)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitContinueStmt(this); }
    std::string toString() const override { return "ContinueStmt"; }
};

class FunctionStmt : public Stmt {
public:
    std::string name;
    std::vector<std::pair<std::string, Type>> parameters;
    std::string returnType;
    std::vector<Stmt::Ptr> body;
    bool isAsync;
    bool isGenerator;
    bool isConstructor;
    bool isStatic;
    
    FunctionStmt(SourceLocation loc, std::string n, std::vector<std::pair<std::string, Type>> params,
                 std::string rt, std::vector<Stmt::Ptr> b,
                 bool async = false, bool gen = false, bool constr = false, bool stat = false)
        : Stmt(loc), name(std::move(n)), parameters(std::move(params)),
          returnType(std::move(rt)), body(std::move(b)), isAsync(async),
          isGenerator(gen), isConstructor(constr), isStatic(stat) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitFunctionStmt(this); }
    std::string toString() const override { return "FunctionStmt(" + name + ")"; }
};

class ClassStmt : public Stmt {
public:
    std::string name;
    std::vector<Expr::Ptr> superClass;
    std::vector<std::pair<std::string, Type>> typeParams;
    std::vector<std::pair<std::string, Type>> extendsTypes;
    std::vector<std::string> implements;
    std::vector<std::shared_ptr<FunctionStmt>> methods;
    std::vector<std::shared_ptr<VarDeclStmt>> fields;
    bool isAbstract;
    bool isFinal;
    
    ClassStmt(SourceLocation loc, std::string n, std::vector<Expr::Ptr> super,
              std::vector<std::pair<std::string, Type>> params,
              std::vector<std::pair<std::string, Type>> extends,
              std::vector<std::string> impl, std::vector<std::shared_ptr<FunctionStmt>> methods,
              std::vector<std::shared_ptr<VarDeclStmt>> fields, bool abstract = false, bool final = false)
        : Stmt(loc), name(std::move(n)), superClass(std::move(super)),
          typeParams(std::move(params)), extendsTypes(std::move(extends)),
          implements(std::move(impl)), methods(std::move(methods)),
          fields(std::move(fields)), isAbstract(abstract), isFinal(final) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitClassStmt(this); }
    std::string toString() const override { return "ClassStmt(" + name + ")"; }
};

class VarDeclStmt : public Stmt {
public:
    std::string name;
    Type type;
    Expr::Ptr initializer;
    bool isConst;
    bool isLet;
    
    VarDeclStmt(SourceLocation loc, std::string n, Type t, Expr::Ptr init,
                bool constDecl = false, bool letDecl = true)
        : Stmt(loc), name(std::move(n)), type(t), initializer(std::move(init)),
          isConst(constDecl), isLet(letDecl) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitVarDeclStmt(this); }
    std::string toString() const override {
        std::string kind = isConst ? "const" : (isLet ? "let" : "var");
        return kind + "(" + name + ")";
    }
};

class ConstDeclStmt : public Stmt {
public:
    std::string name;
    Expr::Ptr value;
    
    ConstDeclStmt(SourceLocation loc, std::string n, Expr::Ptr v)
        : Stmt(loc), name(std::move(n)), value(std::move(v)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitConstDeclStmt(this); }
    std::string toString() const override { return "ConstDeclStmt(" + name + ")"; }
};

class ImportStmt : public Stmt {
public:
    std::string moduleName;
    std::vector<std::pair<std::string, std::string>> namedImports;
    std::string defaultImport;
    std::string namespaceImport;
    
    ImportStmt(SourceLocation loc, std::string mod, std::vector<std::pair<std::string, std::string>> named,
               std::string def = "", std::string ns = "")
        : Stmt(loc), moduleName(std::move(mod)), namedImports(std::move(named)),
          defaultImport(std::move(def)), namespaceImport(std::move(ns)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitImportStmt(this); }
    std::string toString() const override { return "ImportStmt(" + moduleName + ")"; }
};

class ExportStmt : public Stmt {
public:
    bool isDefault;
    std::variant<std::shared_ptr<FunctionStmt>, std::shared_ptr<ClassStmt>, std::shared_ptr<VarDeclStmt>, Expr::Ptr> declaration;
    std::vector<std::string> namedExports;
    std::string exportAllFrom;
    
    ExportStmt(SourceLocation loc, bool def, std::variant<std::shared_ptr<FunctionStmt>, std::shared_ptr<ClassStmt>, std::shared_ptr<VarDeclStmt>, Expr::Ptr> decl,
               std::vector<std::string> named = {}, std::string allFrom = "")
        : Stmt(loc), isDefault(def), declaration(std::move(decl)),
          namedExports(std::move(named)), exportAllFrom(std::move(allFrom)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitExportStmt(this); }
    std::string toString() const override { return "ExportStmt"; }
};

class TryStmt : public Stmt {
public:
    Stmt::Ptr tryBlock;
    std::pair<std::string, Stmt::Ptr> catchClause;
    Stmt::Ptr finallyBlock;
    
    TryStmt(SourceLocation loc, Stmt::Ptr tryBlk, std::pair<std::string, Stmt::Ptr> catchBlk, Stmt::Ptr finallyBlk = nullptr)
        : Stmt(loc), tryBlock(std::move(tryBlk)), catchClause(std::move(catchBlk)), finallyBlock(std::move(finallyBlk)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitTryStmt(this); }
    std::string toString() const override { return "TryStmt"; }
};

class ThrowStmt : public Stmt {
public:
    Expr::Ptr value;
    
    ThrowStmt(SourceLocation loc, Expr::Ptr v)
        : Stmt(loc), value(std::move(v)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitThrowStmt(this); }
    std::string toString() const override { return "ThrowStmt(...)"; }
};

class SwitchStmt : public Stmt {
public:
    Expr::Ptr expression;
    struct Case {
        Expr::Ptr condition;
        std::vector<Stmt::Ptr> statements;
    };
    std::vector<Case> cases;
    
    SwitchStmt(SourceLocation loc, Expr::Ptr expr, std::vector<Case> cs)
        : Stmt(loc), expression(std::move(expr)), cases(std::move(cs)) {}
    
    void accept(StatementVisitor& visitor) override { visitor.visitSwitchStmt(this); }
    std::string toString() const override { return "SwitchStmt"; }
};

// ============ PROGRAM ============

struct Program {
    std::string source;
    std::vector<Stmt::Ptr> statements;
    std::string filename;
    
    Program(std::string src, std::vector<Stmt::Ptr> stmts, std::string fn = "")
        : source(std::move(src)), statements(std::move(stmts)), filename(std::move(fn)) {}
};

} // namespace hs

#endif // HARVIS_AST_AST_H
