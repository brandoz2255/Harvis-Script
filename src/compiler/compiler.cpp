#include "compiler.h"
#include <cstdio>

namespace hs {

Compiler::Compiler() {
    // Initialize with a global scope
    scopes.emplace_back();
}

void Compiler::compile(Program& program) {
    for (const auto& stmt : program.statements) {
        stmt->accept(*this);
    }
    emitReturn();
}

void Compiler::reportError(SourceLocation loc, const std::string& message) {
    errorCount++;
    fprintf(stderr, "%s:%d: error: %s\n", loc.filename.c_str(), loc.line, message.c_str());
}

void Compiler::beginScope() { scopes.emplace_back(); }

void Compiler::endScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

void Compiler::declareVariable(const std::string& name) {
    if (!scopes.empty()) {
        auto& scope = scopes.back();
        for (const auto& s : scope) {
            if (s.name == name) {
                reportError({{}, 0, 0}, "Variable already declared in this scope.");
                return;
            }
        }
        scope.push_back({name, static_cast<int>(scopes.size()), localCount++, true, false});
    }
}

void Compiler::defineVariable(const std::string& name) {
    if (!scopes.empty()) {
        auto& scope = scopes.back();
        for (auto& s : scope) {
            if (s.name == name) {
                s.isLocal = false;  // Now initialized
                return;
            }
        }
    }
}

int Compiler::resolveLocal(const std::string& name, bool errorIfNotFound) {
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; i--) {
        for (const auto& s : scopes[i]) {
            if (s.name == name) {
                return s.slot;  // Use the slot from the symbol
            }
        }
    }
    if (errorIfNotFound) {
        reportError({{}, 0, 0}, "Undefined variable: " + name);
    }
    return -1;
}

void Compiler::emitByte(Opcode op) {
    chunk.write(static_cast<uint8_t>(op), 0);
}

void Compiler::emitByte(uint8_t byte) {
    chunk.write(byte, 0);
}

void Compiler::emitBytes(Opcode op, uint8_t arg) {
    chunk.write(static_cast<uint8_t>(op), 0);
    chunk.write(arg, 0);
}

void Compiler::emitJump(Opcode op, int& jumpOffset) {
    emitByte(op);
    chunk.code.push_back(0);  // Placeholder
    chunk.code.push_back(0);
    jumpOffset = static_cast<int>(chunk.code.size()) - 2;
}

void Compiler::emitReturn() {
    emitByte(Opcode::OP_RETURN);
}

void Compiler::emitCall(int argCount) {
    emitBytes(Opcode::OP_CALL, static_cast<uint8_t>(argCount));
}

void Compiler::emitConstant(Value value) {
    if (value.isNumber()) {
        int index = static_cast<int>(chunk.constants.size());
        chunk.constants.push_back(std::move(value));
        emitByte(Opcode::OP_CONST_NUMBER);
        emitByte(static_cast<uint8_t>(index));
    } else if (value.isString()) {
        int index = static_cast<int>(chunk.constants.size());
        chunk.constants.push_back(std::move(value));
        emitByte(Opcode::OP_CONST_STRING);
        emitByte(static_cast<uint8_t>(index));
    } else if (value.isBoolean()) {
        emitByte(value.toBool() ? Opcode::OP_CONST_TRUE : Opcode::OP_CONST_FALSE);
    } else if (value.isNull()) {
        emitByte(Opcode::OP_CONST_NULL);
    } else {
        emitByte(Opcode::OP_CONST_UNDEFINED);
    }
}

void Compiler::emitGetLocal(int slot) {
    emitBytes(Opcode::OP_GET_LOCAL, static_cast<uint8_t>(slot));
}

void Compiler::emitSetLocal(int slot) {
    emitBytes(Opcode::OP_SET_LOCAL, static_cast<uint8_t>(slot));
}

// ============ EXPRESSION VISITORS ============

void Compiler::visitLiteralExpr(LiteralExpr* expr) {
    emitConstant(expr->value);
}

void Compiler::visitIdentifierExpr(IdentifierExpr* expr) {
    int slot = resolveLocal(expr->name, false);
    if (slot >= 0) {
        emitGetLocal(slot);
    } else {
        int constantIndex = chunk.addStringConstant(expr->name);
        emitByte(Opcode::OP_GET_GLOBAL);
        emitByte(static_cast<uint8_t>(constantIndex));
    }
}

void Compiler::visitBinaryExpr(BinaryExpr* expr) {
    expr->left->accept(*this);
    expr->right->accept(*this);
    
    switch (expr->op) {
        case BinaryOp::PLUS: emitByte(Opcode::OP_ADD); break;
        case BinaryOp::MINUS: emitByte(Opcode::OP_SUBTRACT); break;
        case BinaryOp::STAR: emitByte(Opcode::OP_MULTIPLY); break;
        case BinaryOp::SLASH: emitByte(Opcode::OP_DIVIDE); break;
        case BinaryOp::PERCENT: emitByte(Opcode::OP_MODULO); break;
        case BinaryOp::EQUAL_EQUAL: emitByte(Opcode::OP_EQUAL); break;
        case BinaryOp::BANG_EQUAL: emitByte(Opcode::OP_NOT_EQUAL); break;
        case BinaryOp::LESS: emitByte(Opcode::OP_LESS); break;
        case BinaryOp::LESS_EQUAL: emitByte(Opcode::OP_LESS_EQUAL); break;
        case BinaryOp::GREATER: emitByte(Opcode::OP_GREATER); break;
        case BinaryOp::GREATER_EQUAL: emitByte(Opcode::OP_GREATER_EQUAL); break;
        default: break;
    }
}

void Compiler::visitUnaryExpr(UnaryExpr* expr) {
    expr->operand->accept(*this);
    
    switch (expr->op) {
        case UnaryOp::MINUS: emitByte(Opcode::OP_NEGATE); break;
        case UnaryOp::BANG: emitByte(Opcode::OP_NOT); break;
        default: break;
    }
}

void Compiler::visitGroupingExpr(GroupingExpr* expr) {
    expr->expression->accept(*this);
}

void Compiler::visitCallExpr(CallExpr* expr) {
    expr->callee->accept(*this);
    int argCount = 0;
    for (const auto& arg : expr->arguments) {
        arg->accept(*this);
        argCount++;
    }
    emitCall(argCount);
}

void Compiler::visitArrayLiteralExpr(ArrayLiteralExpr* expr) {
    int count = 0;
    for (const auto& elem : expr->elements) {
        elem->accept(*this);
        count++;
    }
    emitBytes(Opcode::OP_NEW_ARRAY, static_cast<uint8_t>(count));
}

void Compiler::visitObjectLiteralExpr(ObjectLiteralExpr* expr) {
    int count = 0;
    for (const auto& prop : expr->properties) {
        chunk.writeString(prop.name, 0);
        prop.value->accept(*this);
        count++;
    }
    emitBytes(Opcode::OP_NEW_OBJECT, static_cast<uint8_t>(count));
}

void Compiler::visitIndexExpr(IndexExpr* expr) {
    expr->object->accept(*this);
    expr->index->accept(*this);
    emitByte(Opcode::OP_GET_INDEX);
}

void Compiler::visitDotExpr(DotExpr* expr) {
    expr->object->accept(*this);
    chunk.writeString(expr->property, 0);
    emitByte(Opcode::OP_GET_PROPERTY);
}

void Compiler::visitAssignExpr(AssignExpr* expr) {
    if (auto ident = std::dynamic_pointer_cast<IdentifierExpr>(expr->target)) {
        int slot = resolveLocal(ident->name, false);
        expr->value->accept(*this);
        if (slot > 0) {
            emitSetLocal(slot);
        } else {
            emitByte(Opcode::OP_SET_GLOBAL);
            chunk.writeString(ident->name, 0);
        }
    } else if (auto dot = std::dynamic_pointer_cast<DotExpr>(expr->target)) {
        dot->object->accept(*this);
        chunk.writeString(dot->property, 0);
        expr->value->accept(*this);
        emitByte(Opcode::OP_SET_PROPERTY);
    } else if (auto index = std::dynamic_pointer_cast<IndexExpr>(expr->target)) {
        index->object->accept(*this);
        index->index->accept(*this);
        expr->value->accept(*this);
        emitByte(Opcode::OP_SET_INDEX);
    }
}

void Compiler::visitLogicalExpr(LogicalExpr* expr) {
    if (expr->isAnd) {
        expr->left->accept(*this);
        int jumpIfFalse = -1;
        emitJump(Opcode::OP_JUMP_IF_FALSE, jumpIfFalse);
        chunk.code.pop_back(); chunk.code.pop_back();
        emitByte(Opcode::OP_POP);
        expr->right->accept(*this);
        int jumpBack = -1;
        emitJump(Opcode::OP_JUMP, jumpBack);
        // Patch forward jump
        int current = static_cast<int>(chunk.code.size());
        (void)current;
        // This is a simplified version
    } else {
        expr->left->accept(*this);
        int jumpIfTrue = -1;
        emitJump(Opcode::OP_JUMP_IF_TRUE, jumpIfTrue);
        chunk.code.pop_back(); chunk.code.pop_back();
        emitByte(Opcode::OP_POP);
        expr->right->accept(*this);
    }
}

void Compiler::visitTernaryExpr(TernaryExpr* expr) {
    expr->condition->accept(*this);
    int jumpIfFalse = -1;
    emitJump(Opcode::OP_JUMP_IF_FALSE, jumpIfFalse);
    chunk.code.pop_back(); chunk.code.pop_back();
    expr->thenExpr->accept(*this);
    int jumpEnd = -1;
    emitJump(Opcode::OP_JUMP, jumpEnd);
    chunk.code.pop_back(); chunk.code.pop_back();
    // Patch forward
    int falseOffset = static_cast<int>(chunk.code.size()) - jumpIfFalse - 1;
    chunk.code[jumpIfFalse] = static_cast<uint8_t>(falseOffset);
    expr->elseExpr->accept(*this);
    // Patch end jump
    int endOffset = static_cast<int>(chunk.code.size()) - jumpEnd - 1;
    chunk.code[jumpEnd] = static_cast<uint8_t>(endOffset);
}

void Compiler::visitLambdaExpr(LambdaExpr* expr) {
    (void)expr;
    // Simplified: just emit a placeholder
    emitByte(Opcode::OP_NOP);
}

void Compiler::visitNewExpr(NewExpr* expr) {
    expr->callee->accept(*this);
    int count = 0;
    for (const auto& arg : expr->arguments) {
        arg->accept(*this);
        count++;
    }
    emitCall(count);
    emitByte(Opcode::OP_NOP);  // Placeholder for "new"
}

void Compiler::visitThisExpr(ThisExpr* expr) {
    (void)expr;
    emitGetLocal(0);  // 'this' is always at slot 0
}

void Compiler::visitSuperExpr(SuperExpr* expr) {
    (void)expr;
    emitByte(Opcode::OP_NOP);  // Placeholder
}

void Compiler::visitTypeCastExpr(TypeCastExpr* expr) {
    expr->expression->accept(*this);
}

// ============ STATEMENT VISITORS ============

void Compiler::visitExpressionStmt(ExpressionStmt* stmt) {
    stmt->expression->accept(*this);
    emitByte(Opcode::OP_POP);
}

void Compiler::visitBlockStmt(BlockStmt* stmt) {
    beginScope();
    for (const auto& s : stmt->statements) {
        s->accept(*this);
    }
    endScope();
}

void Compiler::visitIfStmt(IfStmt* stmt) {
    stmt->condition->accept(*this);
    int jumpIfFalse = -1;
    emitJump(Opcode::OP_JUMP_IF_FALSE, jumpIfFalse);
    chunk.code.pop_back(); chunk.code.pop_back();
    stmt->thenBranch->accept(*this);
    int jumpEnd = -1;
    emitJump(Opcode::OP_JUMP, jumpEnd);
    chunk.code.pop_back(); chunk.code.pop_back();
    
    // Patch forward
    int falseOffset = static_cast<int>(chunk.code.size()) - jumpIfFalse - 1;
    chunk.code[jumpIfFalse] = static_cast<uint8_t>(falseOffset);
    
    if (stmt->elseBranch) {
        stmt->elseBranch->accept(*this);
    }
    
    // Patch end jump
    int endOffset = static_cast<int>(chunk.code.size()) - jumpEnd - 1;
    chunk.code[jumpEnd] = static_cast<uint8_t>(endOffset);
}

void Compiler::visitWhileStmt(WhileStmt* stmt) {
    int loopStart = static_cast<int>(chunk.code.size());
    stmt->condition->accept(*this);
    int endJump = -1;
    emitJump(Opcode::OP_JUMP_IF_FALSE, endJump);
    chunk.code.pop_back(); chunk.code.pop_back();
    stmt->body->accept(*this);
    emitJump(Opcode::OP_LOOP, loopStart);
    chunk.code[loopStart + 1] = static_cast<uint8_t>(chunk.code.size());
    chunk.code[loopStart + 2] = static_cast<uint8_t>(chunk.code.size() >> 8);
    
    int endOffset = static_cast<int>(chunk.code.size()) - endJump - 1;
    chunk.code[endJump] = static_cast<uint8_t>(endOffset);
    chunk.code[endJump + 1] = static_cast<uint8_t>(endOffset >> 8);
}

void Compiler::visitForStmt(ForStmt* stmt) {
    int loopStart = static_cast<int>(chunk.code.size());
    
    // Init
    if (std::holds_alternative<Expr::Ptr>(stmt->init)) {
        if (auto expr = std::get_if<Expr::Ptr>(&stmt->init)) {
            if (*expr) (*expr)->accept(*this);
            emitByte(Opcode::OP_POP);
        }
    } else if (std::holds_alternative<std::shared_ptr<VarDeclStmt>>(stmt->init)) {
        if (auto var = std::get_if<std::shared_ptr<VarDeclStmt>>(&stmt->init)) {
            if (*var) (*var)->accept(*this);
        }
    }
    
    // Condition
    if (stmt->condition) {
        stmt->condition->accept(*this);
    }
    int endJump = -1;
    emitJump(Opcode::OP_JUMP_IF_FALSE, endJump);
    chunk.code.pop_back(); chunk.code.pop_back();
    
    // Body
    stmt->body->accept(*this);
    
    // Increment
    if (stmt->increment) {
        stmt->increment->accept(*this);
        emitByte(Opcode::OP_POP);
    }
    
    // Loop back
    emitJump(Opcode::OP_LOOP, loopStart);
    
    // Patch end jump
    int endOffset = static_cast<int>(chunk.code.size()) - endJump - 1;
    chunk.code[endJump] = static_cast<uint8_t>(endOffset);
    chunk.code[endJump + 1] = static_cast<uint8_t>(endOffset >> 8);
}

void Compiler::visitDoWhileStmt(DoWhileStmt* stmt) {
    int loopStart = static_cast<int>(chunk.code.size());
    stmt->body->accept(*this);
    stmt->condition->accept(*this);
    int endJump = -1;
    emitJump(Opcode::OP_JUMP_IF_FALSE, endJump);
    chunk.code.pop_back(); chunk.code.pop_back();
    emitJump(Opcode::OP_LOOP, loopStart);
    
    int endOffset = static_cast<int>(chunk.code.size()) - endJump - 1;
    chunk.code[endJump] = static_cast<uint8_t>(endOffset);
    chunk.code[endJump + 1] = static_cast<uint8_t>(endOffset >> 8);
}

void Compiler::visitReturnStmt(ReturnStmt* stmt) {
    if (stmt->value) {
        stmt->value->accept(*this);
    } else {
        emitByte(Opcode::OP_CONST_NULL);
    }
    emitByte(Opcode::OP_RETURN);
}

void Compiler::visitBreakStmt(BreakStmt* stmt) {
    (void)stmt;
    emitByte(Opcode::OP_NOP);  // Placeholder - needs proper loop tracking
}

void Compiler::visitContinueStmt(ContinueStmt* stmt) {
    (void)stmt;
    emitByte(Opcode::OP_NOP);  // Placeholder
}

void Compiler::visitFunctionStmt(FunctionStmt* stmt) {
    // Simplified function compilation
    declareVariable(stmt->name);
    emitByte(Opcode::OP_NOP);  // Placeholder for function definition
}

void Compiler::visitClassStmt(ClassStmt* stmt) {
    declareVariable(stmt->name);
    emitByte(Opcode::OP_NEW_CLASS);
    chunk.writeString(stmt->name, 0);
}

void Compiler::visitVarDeclStmt(VarDeclStmt* stmt) {
    declareVariable(stmt->name);
    // Get the slot before compiling initializer
    int slot = resolveLocal(stmt->name, false);
    
    if (stmt->initializer) {
        stmt->initializer->accept(*this);
    } else {
        emitByte(Opcode::OP_CONST_NULL);
    }
    
    // Store in local slot if found (slot >= 0), otherwise global
    if (slot >= 0) {
        emitSetLocal(slot);
    } else {
        emitByte(Opcode::OP_SET_GLOBAL);
        chunk.writeString(stmt->name, 0);
    }
}

void Compiler::visitConstDeclStmt(ConstDeclStmt* stmt) {
    declareVariable(stmt->name);
    stmt->value->accept(*this);
    defineVariable(stmt->name);
}

void Compiler::visitImportStmt(ImportStmt* stmt) {
    (void)stmt;
    emitByte(Opcode::OP_NOP);
}

void Compiler::visitExportStmt(ExportStmt* stmt) {
    (void)stmt;
    emitByte(Opcode::OP_NOP);
}

void Compiler::visitTryStmt(TryStmt* stmt) {
    emitByte(Opcode::OP_TRY);
    stmt->tryBlock->accept(*this);
    if (stmt->catchClause.second) {
        emitByte(Opcode::OP_CATCH);
        stmt->catchClause.second->accept(*this);
    }
    if (stmt->finallyBlock) {
        stmt->finallyBlock->accept(*this);
    }
}

void Compiler::visitThrowStmt(ThrowStmt* stmt) {
    stmt->value->accept(*this);
    emitByte(Opcode::OP_THROW);
}

void Compiler::visitSwitchStmt(SwitchStmt* stmt) {
    stmt->expression->accept(*this);
    // Simplified switch - just NOP for now
    emitByte(Opcode::OP_NOP);
}

} // namespace hs
