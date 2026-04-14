#ifndef HARVIS_AST_AST_VISITOR_H
#define HARVIS_AST_AST_VISITOR_H

#include "ast.h"

namespace hs {

// Concrete visitor implementations will go here
// For now, just a placeholder

class DebugExpressionVisitor : public Visitor {
public:
    void visitLiteralExpr(LiteralExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitIdentifierExpr(IdentifierExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitBinaryExpr(BinaryExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitUnaryExpr(UnaryExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitGroupingExpr(GroupingExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitCallExpr(CallExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitArrayLiteralExpr(ArrayLiteralExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitObjectLiteralExpr(ObjectLiteralExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitIndexExpr(IndexExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitDotExpr(DotExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitAssignExpr(AssignExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitLogicalExpr(LogicalExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitTernaryExpr(TernaryExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitLambdaExpr(LambdaExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitNewExpr(NewExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitThisExpr(ThisExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitSuperExpr(SuperExpr* expr) override { std::cout << expr->toString() << std::endl; }
    void visitTypeCastExpr(TypeCastExpr* expr) override { std::cout << expr->toString() << std::endl; }
};

class DebugStatementVisitor : public StatementVisitor {
public:
    void visitExpressionStmt(ExpressionStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitBlockStmt(BlockStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitIfStmt(IfStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitWhileStmt(WhileStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitForStmt(ForStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitDoWhileStmt(DoWhileStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitReturnStmt(ReturnStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitBreakStmt(BreakStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitContinueStmt(ContinueStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitFunctionStmt(FunctionStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitClassStmt(ClassStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitVarDeclStmt(VarDeclStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitConstDeclStmt(ConstDeclStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitImportStmt(ImportStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitExportStmt(ExportStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitTryStmt(TryStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitThrowStmt(ThrowStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
    void visitSwitchStmt(SwitchStmt* stmt) override { std::cout << stmt->toString() << std::endl; }
};

} // namespace hs

#endif // HARVIS_AST_AST_VISITOR_H
