#ifndef HARVIS_COMPILER_COMPILER_H
#define HARVIS_COMPILER_COMPILER_H

#include "chunk.h"
#include "../ast/ast.h"
#include <stack>
#include <unordered_map>

namespace hs {

// Symbol table entry
struct Symbol {
    std::string name;
    int depth;
    int slot;
    bool isLocal;
    bool isConstant;
};

// Compiler implements both Expression and Statement Visitor
class Compiler : public Visitor, public StatementVisitor {
public:
    Compiler();
    
    void compile(Program& program);
    Chunk* getChunk() { return &chunk; }
    bool hasErrors() const { return errorCount > 0; }
    void reportError(SourceLocation loc, const std::string& message);

private:
    Chunk chunk;
    std::vector<CompiledFunction> functions;
    std::vector<std::vector<Symbol>> scopes;
    int errorCount = 0;
    int localCount = 0;
    bool inClass = false;
    bool inFunction = false;
    int loopDepth = -1;
    std::vector<int> loopEndJumps;
    std::vector<std::unordered_map<std::string, Type>> typeParamBindings;
    
    CompiledFunction* currentFunction() {
        if (functions.empty()) return nullptr;
        return &functions.back();
    }
    
    // Scopes
    void beginScope();
    void endScope();
    
    // Variable declarations
    void declareVariable(const std::string& name);
    void defineVariable(const std::string& name);
    void markInitialized();
    
    // Variable access
    int resolveLocal(const std::string& name, bool errorIfNotFound);
    int resolveLocalDepth(const std::string& name);
    void emitGetLocal(int slot);
    void emitSetLocal(int slot);
    
    // Bytecode emission
    void emitByte(Opcode op);
    void emitByte(uint8_t byte);
    void emitBytes(Opcode op, uint8_t arg);
    void emitJump(Opcode op, int& jumpOffset);
    void emitReturn();
    void emitCall(int argCount);
    void patchJump(int jumpOffset, int target);
    
    // Helper methods
    void emitConstant(Value value);
    int makeConstant(Value value);
    int reserveSlot();
    int currentScope() const { return static_cast<int>(scopes.size()); }
    
    // Loop tracking
    void enterLoop();
    void exitLoop();
    void emitJumpToLoopEnd(int& jumpOffset);
    void patchLoopEnd(int jumpOffset, int target);
    
    // Type parameter handling
    void pushTypeParams(const std::vector<std::string>& params);
    void popTypeParams();
    std::string resolveTypeParam(const std::string& name);
    std::string mangleTypeName(const std::string& base, const std::vector<Type>& args);
    std::string mangleTypeName(const std::string& base, const std::vector<std::string>& args);
    
    // Expression visitors
    void visitLiteralExpr(LiteralExpr* expr) override;
    void visitIdentifierExpr(IdentifierExpr* expr) override;
    void visitBinaryExpr(BinaryExpr* expr) override;
    void visitUnaryExpr(UnaryExpr* expr) override;
    void visitGroupingExpr(GroupingExpr* expr) override;
    void visitCallExpr(CallExpr* expr) override;
    void visitArrayLiteralExpr(ArrayLiteralExpr* expr) override;
    void visitObjectLiteralExpr(ObjectLiteralExpr* expr) override;
    void visitIndexExpr(IndexExpr* expr) override;
    void visitDotExpr(DotExpr* expr) override;
    void visitAssignExpr(AssignExpr* expr) override;
    void visitLogicalExpr(LogicalExpr* expr) override;
    void visitTernaryExpr(TernaryExpr* expr) override;
    void visitLambdaExpr(LambdaExpr* expr) override;
    void visitNewExpr(NewExpr* expr) override;
    void visitThisExpr(ThisExpr* expr) override;
    void visitSuperExpr(SuperExpr* expr) override;
    void visitTypeCastExpr(TypeCastExpr* expr) override;
    
    // Statement visitors
    void visitExpressionStmt(ExpressionStmt* stmt) override;
    void visitBlockStmt(BlockStmt* stmt) override;
    void visitIfStmt(IfStmt* stmt) override;
    void visitWhileStmt(WhileStmt* stmt) override;
    void visitForStmt(ForStmt* stmt) override;
    void visitDoWhileStmt(DoWhileStmt* stmt) override;
    void visitReturnStmt(ReturnStmt* stmt) override;
    void visitBreakStmt(BreakStmt* stmt) override;
    void visitContinueStmt(ContinueStmt* stmt) override;
    void visitFunctionStmt(FunctionStmt* stmt) override;
    void visitClassStmt(ClassStmt* stmt) override;
    void visitVarDeclStmt(VarDeclStmt* stmt) override;
    void visitConstDeclStmt(ConstDeclStmt* stmt) override;
    void visitImportStmt(ImportStmt* stmt) override;
    void visitExportStmt(ExportStmt* stmt) override;
    void visitPackageStmt(PackageStmt* stmt) override;
    void visitTryStmt(TryStmt* stmt) override;
    void visitThrowStmt(ThrowStmt* stmt) override;
    void visitSwitchStmt(SwitchStmt* stmt) override;
    void visitStructDeclStmt(StructDeclStmt* stmt) override;
    void visitDeferStmt(DeferStmt* stmt) override;
    void visitInterfaceDeclStmt(InterfaceDeclStmt* stmt) override;
    void visitStructInstantiationExpr(StructInstantiationExpr* expr) override;
    void visitPanicExpr(PanicExpr* expr) override;
    void visitRecoverExpr(RecoverExpr* expr) override;
    void visitChannelExpr(ChannelExpr* expr) override;
    void visitSendExpr(SendExpr* expr) override;
    void visitReceiveExpr(ReceiveExpr* expr) override;
    void visitMutexExpr(MutexExpr* expr) override;
   void visitWaitGroupExpr(WaitGroupExpr* expr) override;
    void visitGoStmt(GoStmt* stmt) override;
    void visitSelectStmt(SelectStmt* stmt) override;
    void visitTypeAssertExpr(TypeAssertExpr* expr) override;
    void visitRangeStmt(RangeStmt* stmt) override;
    void visitTypeSwitchStmt(TypeSwitchStmt* stmt) override;
};

} // namespace hs

#endif // HARVIS_COMPILER_COMPILER_H
