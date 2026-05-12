#include "compiler.h"
#include "../vm/object.h"
#include <cstdio>
#include <algorithm>

namespace hs {

Compiler::Compiler() {
    // Initialize with a global scope
    scopes.emplace_back();
}

void Compiler::compile(Program& program) {
    for (const auto& stmt : program.statements) {
        stmt->accept(*this);
    }
    emitByte(Opcode::OP_DEFERRED_RETURN);
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

int Compiler::resolveLocalDepth(const std::string& name) {
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; i--) {
        for (const auto& s : scopes[i]) {
            if (s.name == name) {
                return i;
            }
        }
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

void Compiler::emitJumpToLoopEnd(int& jumpOffset) {
    if (loopDepth >= 0) {
        emitJump(Opcode::OP_JUMP, jumpOffset);
        loopEndJumps.push_back(jumpOffset);
    }
}

void Compiler::patchLoopEnd(int jumpOffset, int target) {
     int offset = target - (jumpOffset + 2);
     chunk.code[jumpOffset] = static_cast<uint8_t>(offset);
     chunk.code[jumpOffset + 1] = static_cast<uint8_t>(offset >> 8);
 }
 
 void Compiler::patchJump(int jumpOffset, int target) {
     int offset = target - (jumpOffset + 2);
     chunk.code[jumpOffset] = static_cast<uint8_t>(offset);
     chunk.code[jumpOffset + 1] = static_cast<uint8_t>(offset >> 8);
 }

void Compiler::enterLoop() {
    loopDepth++;
}

void Compiler::exitLoop() {
    loopDepth--;
}

void Compiler::emitReturn() {
    emitByte(Opcode::OP_RETURN);
}

void Compiler::emitCall(int argCount) {
    emitBytes(Opcode::OP_CALL, static_cast<uint8_t>(argCount));
}

int Compiler::makeConstant(Value value) {
    int index = static_cast<int>(chunk.constants.size());
    chunk.constants.push_back(std::move(value));
    return index;
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
        int constantIndex = chunk.findOrAddStringConstant(expr->name);
        emitByte(Opcode::OP_GET_GLOBAL);
        emitByte(static_cast<uint8_t>(constantIndex));
    }
}

void Compiler::visitBinaryExpr(BinaryExpr* expr) {
     if (expr->op != BinaryOp::NULLISH_COALESCE) {
         expr->left->accept(*this);
         expr->right->accept(*this);
     }
     
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
         case BinaryOp::AMPERSAND: emitByte(Opcode::OP_BITWISE_AND); break;
         case BinaryOp::PIPE: emitByte(Opcode::OP_BITWISE_OR); break;
         case BinaryOp::CARET: emitByte(Opcode::OP_BITWISE_XOR); break;
         case BinaryOp::LESS_LESS: emitByte(Opcode::OP_SHIFT_LEFT); break;
         case BinaryOp::GREATER_GREATER: emitByte(Opcode::OP_SHIFT_RIGHT); break;
         case BinaryOp::NULLISH_COALESCE: {
              // a ?? b: if a is not null/undefined, return a; otherwise return b
              expr->left->accept(*this);
              expr->right->accept(*this);
              // Stack: left, right
              // OP_NULLISH_COALESCE: pop both, push left if not null, else push right
              emitByte(Opcode::OP_NULLISH_COALESCE);
              break;
          }
         default: break;
     }
 }

void Compiler::visitUnaryExpr(UnaryExpr* expr) {
    expr->operand->accept(*this);
    
      switch (expr->op) {
        case UnaryOp::MINUS: emitByte(Opcode::OP_NEGATE); break;
        case UnaryOp::BANG: emitByte(Opcode::OP_NOT); break;
        case UnaryOp::TILDE: emitByte(Opcode::OP_BITWISE_NOT); break;
        default: break;
    }
}

void Compiler::visitGroupingExpr(GroupingExpr* expr) {
    expr->expression->accept(*this);
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
     if (expr->optional) {
         emitByte(Opcode::OP_GET_INDEX_OPTIONAL);
     } else {
         emitByte(Opcode::OP_GET_INDEX);
     }
 }

 void Compiler::visitDotExpr(DotExpr* expr) {
     expr->object->accept(*this);
     int nameIndex = chunk.findOrAddStringConstant(expr->property);
     if (expr->optional) {
         emitByte(Opcode::OP_GET_FIELD_OPTIONAL);
         emitByte(static_cast<uint8_t>(nameIndex));
     } else {
         emitByte(Opcode::OP_GET_FIELD);
         emitByte(static_cast<uint8_t>(nameIndex));
     }
 }

void Compiler::visitAssignExpr(AssignExpr* expr) {
    if (auto ident = std::dynamic_pointer_cast<IdentifierExpr>(expr->target)) {
        int slot = resolveLocal(ident->name, false);
        expr->value->accept(*this);
        if (slot >= 0) {
            emitSetLocal(slot);
        } else {
            int constantIndex = chunk.findOrAddStringConstant(ident->name);
            emitByte(Opcode::OP_SET_GLOBAL);
            emitByte(static_cast<uint8_t>(constantIndex));
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
        emitByte(Opcode::OP_POP);
        expr->right->accept(*this);
    } else {
        expr->left->accept(*this);
        int jumpIfTrue = -1;
        emitJump(Opcode::OP_JUMP_IF_TRUE, jumpIfTrue);
        emitByte(Opcode::OP_POP);
        expr->right->accept(*this);
    }
}

void Compiler::visitTernaryExpr(TernaryExpr* expr) {
    expr->condition->accept(*this);
    int jumpIfFalse = -1;
    emitJump(Opcode::OP_JUMP_IF_FALSE, jumpIfFalse);
    expr->thenExpr->accept(*this);
    int jumpEnd = -1;
    emitJump(Opcode::OP_JUMP, jumpEnd);
    patchJump(jumpIfFalse, static_cast<int>(chunk.code.size()));
    expr->elseExpr->accept(*this);
    patchJump(jumpEnd, static_cast<int>(chunk.code.size()));
}

void Compiler::visitLambdaExpr(LambdaExpr* expr) {
    Compiler bodyCompiler;
    bodyCompiler.beginScope();
    
    for (const auto& param : expr->parameters) {
        bodyCompiler.declareVariable(param.first);
    }
    
    if (expr->body) {
        expr->body->accept(bodyCompiler);
    }
    for (const auto& blkStmt : expr->block) {
        blkStmt->accept(bodyCompiler);
    }
    bodyCompiler.endScope();
    
    Function* func = new Function(expr->name, bodyCompiler.chunk, 
                                  static_cast<int>(expr->parameters.size()));
    
    int funcIndex = static_cast<int>(chunk.constants.size());
    chunk.constants.push_back(Value(func));
    
    emitByte(Opcode::OP_CLOSURE);
    emitByte(static_cast<uint8_t>(funcIndex));
    emitByte(0);
}

void Compiler::visitNewExpr(NewExpr* expr) {
    // Generic new expression: new Type<T>(args)
    if (!expr->typeArgs.empty()) {
        // Mangle the class name with type args
        std::string calleeName = "unknown";
        if (auto ident = std::dynamic_pointer_cast<IdentifierExpr>(expr->callee)) {
            calleeName = ident->name;
        }
        std::string mangledName = mangleTypeName(calleeName, expr->typeArgs);
        
        // Push type arg names as string constants
        for (const auto& ta : expr->typeArgs) {
            int idx = chunk.findOrAddStringConstant(ta.toString());
            emitByte(Opcode::OP_CONST_STRING);
            emitByte(static_cast<uint8_t>(idx));
        }
        
        // Push the class name
        int nameIdx = chunk.findOrAddStringConstant(mangledName);
        emitByte(Opcode::OP_CONST_STRING);
        emitByte(static_cast<uint8_t>(nameIdx));
        
        // Push constructor name
        int ctorIdx = chunk.findOrAddStringConstant("constructor");
        emitByte(Opcode::OP_CONST_STRING);
        emitByte(static_cast<uint8_t>(ctorIdx));
        
        // Push constructor arguments
        int count = static_cast<int>(expr->arguments.size());
        for (const auto& arg : expr->arguments) {
            arg->accept(*this);
        }
        
        // OP_NEW_CLASS_GENERIC: argCount (low byte), typeArgCount (high byte)
        emitByte(Opcode::OP_NEW_CLASS_GENERIC);
        emitByte(static_cast<uint8_t>(count));
        emitByte(static_cast<uint8_t>(expr->typeArgs.size()));
        return;
    }
    
    // Regular new expression: new Type(args)
    expr->callee->accept(*this);
    
    int nameIdx = chunk.findOrAddStringConstant("constructor");
    emitByte(Opcode::OP_CONST_STRING);
    emitByte(static_cast<uint8_t>(nameIdx));
    emitByte(Opcode::OP_GET_PROPERTY);
    
    int count = static_cast<int>(expr->arguments.size());
    for (const auto& arg : expr->arguments) {
        arg->accept(*this);
    }
    
    emitByte(Opcode::OP_CALL);
    emitByte(static_cast<uint8_t>(count));
    
    emitByte(Opcode::OP_NEW_INSTANCE);
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
    if (!std::dynamic_pointer_cast<AssignExpr>(stmt->expression)) {
        emitByte(Opcode::OP_POP);
    }
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
    stmt->thenBranch->accept(*this);
    int jumpEnd = -1;
    emitJump(Opcode::OP_JUMP, jumpEnd);
    
    patchJump(jumpIfFalse, static_cast<int>(chunk.code.size()));
    patchJump(jumpEnd, static_cast<int>(chunk.code.size()));
}

void Compiler::visitWhileStmt(WhileStmt* stmt) {
    int loopStart = static_cast<int>(chunk.code.size());
    int savedLoopDepth = loopDepth;
    enterLoop();
    
    stmt->condition->accept(*this);
    int endJump = -1;
    emitJump(Opcode::OP_JUMP_IF_FALSE, endJump);
    stmt->body->accept(*this);
    emitJump(Opcode::OP_LOOP, loopStart);
    
    // Patch all loop end jumps (break)
    for (auto& jumpOff : loopEndJumps) {
        int offset = loopStart - (jumpOff + 2);
        chunk.code[jumpOff] = static_cast<uint8_t>(offset);
        chunk.code[jumpOff + 1] = static_cast<uint8_t>(offset >> 8);
    }
    loopEndJumps.clear();
    
    // Patch end jump
    int endOffset = static_cast<int>(chunk.code.size()) - endJump - 2;
    chunk.code[endJump] = static_cast<uint8_t>(endOffset);
    chunk.code[endJump + 1] = static_cast<uint8_t>(endOffset >> 8);
    
   loopDepth = savedLoopDepth;
}

void Compiler::visitForStmt(ForStmt* stmt) {
    int loopStart = static_cast<int>(chunk.code.size());
    int savedLoopDepth = loopDepth;
    enterLoop();
    beginScope();

    // Init
    if (std::holds_alternative<Expr::Ptr>(stmt->init)) {
        if (auto expr = std::get_if<Expr::Ptr>(&stmt->init)) {
            if (*expr) {
                (*expr)->accept(*this);
                if (!std::dynamic_pointer_cast<AssignExpr>(*expr)) {
                    emitByte(Opcode::OP_POP);
                }
            }
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
    
    // Body
    stmt->body->accept(*this);
    
    // Increment
    if (stmt->increment) {
        stmt->increment->accept(*this);
        if (!std::dynamic_pointer_cast<AssignExpr>(stmt->increment)) {
            emitByte(Opcode::OP_POP);
        }
    }
    
    // Loop back
    emitJump(Opcode::OP_LOOP, loopStart);
    
    // Patch all loop end jumps (break)
    for (auto& jumpOff : loopEndJumps) {
        int offset = loopStart - (jumpOff + 2);
        chunk.code[jumpOff] = static_cast<uint8_t>(offset);
        chunk.code[jumpOff + 1] = static_cast<uint8_t>(offset >> 8);
    }
    loopEndJumps.clear();
    
    // Patch end jump
    int endOffset = static_cast<int>(chunk.code.size()) - endJump - 2;
    chunk.code[endJump] = static_cast<uint8_t>(endOffset);
    chunk.code[endJump + 1] = static_cast<uint8_t>(endOffset >> 8);
    
    loopDepth = savedLoopDepth;
    endScope();
}
void Compiler::visitDoWhileStmt(DoWhileStmt* stmt) {
    int loopStart = static_cast<int>(chunk.code.size());
    int savedLoopDepth = loopDepth;
    enterLoop();
    
    stmt->body->accept(*this);
    stmt->condition->accept(*this);
    int endJump = -1;
    emitJump(Opcode::OP_JUMP_IF_FALSE, endJump);
    emitJump(Opcode::OP_LOOP, loopStart);
    
    // Patch all loop end jumps (break)
    for (auto& jumpOff : loopEndJumps) {
        int offset = loopStart - (jumpOff + 2);
        chunk.code[jumpOff] = static_cast<uint8_t>(offset);
        chunk.code[jumpOff + 1] = static_cast<uint8_t>(offset >> 8);
    }
    loopEndJumps.clear();
    
    // Patch end jump
    int endOffset = static_cast<int>(chunk.code.size()) - endJump - 2;
    chunk.code[endJump] = static_cast<uint8_t>(endOffset);
    chunk.code[endJump + 1] = static_cast<uint8_t>(endOffset >> 8);
    
    loopDepth = savedLoopDepth;
    exitLoop();
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
    if (loopDepth < 0) {
        reportError(stmt->location, "break outside of loop");
        return;
    }
    int jumpOffset = -1;
    emitJump(Opcode::OP_JUMP, jumpOffset);
    loopEndJumps.push_back(jumpOffset);
}

void Compiler::visitContinueStmt(ContinueStmt* stmt) {
    (void)stmt;
    if (loopDepth < 0) {
        reportError(stmt->location, "continue outside of loop");
        return;
    }
    int jumpOffset = -1;
    emitJump(Opcode::OP_JUMP, jumpOffset);
    loopEndJumps.push_back(jumpOffset);
}

void Compiler::visitClassStmt(ClassStmt* stmt) {
    declareVariable(stmt->name);
    
    beginScope();
    
    if (!stmt->typeParams.empty()) {
        pushTypeParams(stmt->typeParams);
    }
    
    for (const auto& method : stmt->methods) {
        Compiler methodCompiler;
        methodCompiler.beginScope();
        
        if (!stmt->typeParams.empty()) {
            methodCompiler.pushTypeParams(stmt->typeParams);
        }
        
        for (const auto& param : method->parameters) {
            methodCompiler.declareVariable(param.first);
        }
        
        for (const auto& bodyStmt : method->body) {
            bodyStmt->accept(methodCompiler);
        }
        methodCompiler.endScope();
        
        Function* func = new Function(method->name, methodCompiler.chunk, 
                                      static_cast<int>(method->parameters.size()));
        func->isClassMethod = true;
        
        int funcIndex = static_cast<int>(chunk.constants.size());
        chunk.constants.push_back(Value(func));
        
        emitByte(Opcode::OP_CLOSURE);
        emitByte(static_cast<uint8_t>(funcIndex));
        emitByte(0);
        
        int methodNameIdx = chunk.findOrAddStringConstant(method->name);
        emitByte(Opcode::OP_METHOD);
        emitByte(static_cast<uint8_t>(methodNameIdx));
    }
    
    if (!stmt->typeParams.empty()) {
        popTypeParams();
    }
    
    endScope();
    
    // Emit class creation bytecode (after methods are defined so closures are in constants)
    if (!stmt->typeParams.empty()) {
        std::string mangledName = mangleTypeName(stmt->name, stmt->typeParams);
        
        for (const auto& tp : stmt->typeParams) {
            int idx = chunk.findOrAddStringConstant(tp);
            emitByte(Opcode::OP_CONST_STRING);
            emitByte(static_cast<uint8_t>(idx));
        }
        
        int nameIdx = chunk.findOrAddStringConstant(mangledName);
        emitByte(Opcode::OP_CONST_STRING);
        emitByte(static_cast<uint8_t>(nameIdx));
        
        int ctorIdx = chunk.findOrAddStringConstant("constructor");
        emitByte(Opcode::OP_CONST_STRING);
        emitByte(static_cast<uint8_t>(ctorIdx));
        
        emitByte(Opcode::OP_NEW_CLASS_GENERIC);
        emitByte(0);
        emitByte(static_cast<uint8_t>(stmt->typeParams.size()));
        
        int slot = resolveLocal(stmt->name, false);
        if (slot >= 0) {
            emitSetLocal(slot);
        } else {
            int finalIdx = chunk.findOrAddStringConstant(stmt->name);
            emitByte(Opcode::OP_SET_GLOBAL);
            emitByte(static_cast<uint8_t>(finalIdx));
        }
    } else {
        int nameIdx = chunk.findOrAddStringConstant(stmt->name);
        emitByte(Opcode::OP_NEW_CLASS);
        emitByte(static_cast<uint8_t>(nameIdx));
        
        int slot = resolveLocal(stmt->name, false);
        if (slot >= 0) {
            emitSetLocal(slot);
        } else {
            emitByte(Opcode::OP_SET_GLOBAL);
            emitByte(static_cast<uint8_t>(nameIdx));
        }
    }
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
    bool shorthandImport = stmt->defaultImport.empty() && 
                           stmt->namedImports.empty() && 
                           stmt->namespaceImport.empty();
    std::string varName = stmt->defaultImport;
    if (shorthandImport) {
        size_t lastSlash = stmt->moduleName.find_last_of("/\\");
        size_t start = (lastSlash != std::string::npos) ? lastSlash + 1 : 0;
        size_t dotPos = stmt->moduleName.find('.', start);
        std::string basename = stmt->moduleName.substr(start, dotPos - start);
        for (char& c : basename) {
            if (!std::isalnum(c)) c = '_';
        }
        varName = basename;
    }
    
    int modIdx = chunk.findOrAddStringConstant(stmt->moduleName);
    emitByte(Opcode::OP_CONST_STRING);
    emitByte(static_cast<uint8_t>(modIdx));
    emitByte(Opcode::OP_IMPORT);
    
    if (!stmt->namedImports.empty()) {
        for (const auto& name : stmt->namedImports) {
            emitByte(Opcode::OP_DUP);
            int nameIdx = chunk.findOrAddStringConstant(name.first);
            emitByte(Opcode::OP_CONST_STRING);
            emitByte(static_cast<uint8_t>(nameIdx));
            emitByte(Opcode::OP_GET_PROPERTY);
            int varIdx = chunk.findOrAddStringConstant(name.first);
            emitByte(Opcode::OP_SET_GLOBAL);
            emitByte(static_cast<uint8_t>(varIdx));
        }
        return;
    }
    
    if (!stmt->namespaceImport.empty()) {
        int varIdx = chunk.findOrAddStringConstant(stmt->namespaceImport);
        emitByte(Opcode::OP_SET_GLOBAL);
        emitByte(static_cast<uint8_t>(varIdx));
        return;
    }
    
    if (!stmt->defaultImport.empty()) {
        int varIdx = chunk.findOrAddStringConstant(stmt->defaultImport);
        emitByte(Opcode::OP_SET_GLOBAL);
        emitByte(static_cast<uint8_t>(varIdx));
        return;
    }
    
    if (shorthandImport && !varName.empty()) {
        int varIdx = chunk.findOrAddStringConstant(varName);
        emitByte(Opcode::OP_SET_GLOBAL);
        emitByte(static_cast<uint8_t>(varIdx));
    }
}

void Compiler::visitExportStmt(ExportStmt* stmt) {
    // Handle named exports with 'from' clause
    if (!stmt->namedExports.empty()) {
        if (!stmt->exportAllFrom.empty()) {
            // Re-export from another module
            int modIdx = chunk.findOrAddStringConstant(stmt->exportAllFrom);
            emitByte(Opcode::OP_CONST_STRING);
            emitByte(static_cast<uint8_t>(modIdx));
            emitByte(Opcode::OP_IMPORT);
        }
        // Mark exports
        for (const auto& name : stmt->namedExports) {
            int nameIdx = chunk.findOrAddStringConstant(name);
            emitByte(Opcode::OP_EXPORT);
            emitByte(static_cast<uint8_t>(nameIdx));
        }
        return;
    }
    
    // Handle declaration exports
    bool hasDecl = false;
    std::visit([&hasDecl](const auto& arg) {
        hasDecl = arg != nullptr;
    }, stmt->declaration);
    
    if (hasDecl) {
        std::visit([this](const auto& arg) {
            if (arg) {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, Expr::Ptr>) {
                    arg->accept(*this);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::shared_ptr<FunctionStmt>>) {
                    arg->accept(*this);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::shared_ptr<ClassStmt>>) {
                    arg->accept(*this);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::shared_ptr<VarDeclStmt>>) {
                    arg->accept(*this);
                }
            }
        }, stmt->declaration);
    }
    
    // Mark as exported
    if (!stmt->isDefault) {
        emitByte(Opcode::OP_EXPORT);
    }
}

void Compiler::visitPackageStmt(PackageStmt* stmt) {
    int pkgIdx = chunk.findOrAddStringConstant(stmt->name);
    emitByte(Opcode::OP_PACKAGE);
    emitByte(static_cast<uint8_t>(pkgIdx));
}

void Compiler::visitTryStmt(TryStmt* stmt) {
    // OP_TRY catch_offset (2-byte jump to catch block)
    int tryCatchJump = -1;
    emitJump(Opcode::OP_TRY, tryCatchJump);

    // Try body
    stmt->tryBlock->accept(*this);

    int hasCatch = (stmt->catchClause.second != nullptr);

    if (hasCatch) {
        // Jump over catch block if no exception
        int skipCatchJump = -1;
        emitJump(Opcode::OP_JUMP, skipCatchJump);

        // Patch OP_TRY to point to catch block start
        int catchStart = static_cast<int>(chunk.code.size());
        patchJump(tryCatchJump, catchStart);

        // Emit catch variable binding
        int varIdx = chunk.findOrAddStringConstant(stmt->catchClause.first);
        emitByte(Opcode::OP_CATCH);
        emitByte(static_cast<uint8_t>(varIdx));

        // Catch body
        stmt->catchClause.second->accept(*this);

        // Patch skipCatchJump to point past catch block
        patchJump(skipCatchJump, static_cast<int>(chunk.code.size()));
    } else {
        // No catch block, patch OP_TRY to skip to after try
        patchJump(tryCatchJump, static_cast<int>(chunk.code.size()));
    }

    // Finally block (always runs)
    if (stmt->finallyBlock) {
        stmt->finallyBlock->accept(*this);
    }
}

void Compiler::visitThrowStmt(ThrowStmt* stmt) {
    stmt->value->accept(*this);
    emitByte(Opcode::OP_THROW);
}

 void Compiler::visitSwitchStmt(SwitchStmt* stmt) {
    int savedLoopDepth = loopDepth;
    enterLoop();

    // Save switch value to a temp local so we can compare against each case
    stmt->expression->accept(*this);
    int switchSlot = localCount++;
    emitBytes(Opcode::OP_SET_LOCAL, static_cast<uint8_t>(switchSlot));

    std::vector<int> skipOffsets;  // JUMP_IF_FALSE offsets to patch later
    std::vector<int> caseStarts;   // bytecode position of each case start
    std::vector<int> endJumps;     // JUMP offsets from case bodies to end

    for (int i = 0; i < static_cast<int>(stmt->cases.size()); i++) {
        caseStarts.push_back(static_cast<int>(chunk.code.size()));
        auto& caseStmt = stmt->cases[i];

        if (caseStmt.condition) {
            emitBytes(Opcode::OP_GET_LOCAL, static_cast<uint8_t>(switchSlot));
            caseStmt.condition->accept(*this);
            emitByte(Opcode::OP_EQUAL);

            int skipOffset = -1;
            emitJump(Opcode::OP_JUMP_IF_FALSE, skipOffset);
            skipOffsets.push_back(skipOffset);
        } else {
            skipOffsets.push_back(-1);  // Default case, no skip needed
        }

        // Case body
        for (const auto& s : caseStmt.statements) {
            s->accept(*this);
        }

        // Non-last cases need to jump to end after body
        if (i < static_cast<int>(stmt->cases.size()) - 1) {
            int jumpToEnd = -1;
            emitJump(Opcode::OP_JUMP, jumpToEnd);
            endJumps.push_back(jumpToEnd);
        }
    }

    int switchEnd = static_cast<int>(chunk.code.size());

    // Patch skip offsets to point to next case start or switch end
    for (int i = 0; i < static_cast<int>(skipOffsets.size()); i++) {
        if (skipOffsets[i] < 0) continue;

        int target = (i + 1 < static_cast<int>(caseStarts.size()))
            ? caseStarts[i + 1] : switchEnd;

        patchJump(skipOffsets[i], target);
    }

    // Patch case end jumps to switch end
    for (auto& jumpOff : endJumps) {
        patchJump(jumpOff, switchEnd);
    }

    // Patch break jumps to switch end
    for (auto& jumpOff : loopEndJumps) {
        patchJump(jumpOff, switchEnd);
    }
    loopEndJumps.clear();

    loopDepth = savedLoopDepth;
}

void Compiler::visitDeferStmt(DeferStmt* stmt) {
    Compiler bodyCompiler;
    bodyCompiler.beginScope();
    stmt->expression->accept(bodyCompiler);
    bodyCompiler.emitReturn();
    bodyCompiler.endScope();
    
    Function* func = new Function("defer", bodyCompiler.chunk, 0);
    int funcIndex = static_cast<int>(chunk.constants.size());
    chunk.constants.push_back(Value(func));
    
    emitByte(Opcode::OP_CLOSURE);
    emitByte(static_cast<uint8_t>(funcIndex));
    emitByte(0);
    
    emitByte(Opcode::OP_DEFER);
}

void Compiler::visitInterfaceDeclStmt(InterfaceDeclStmt* stmt) {
    declareVariable(stmt->name);
    // Interfaces are compile-time only, no bytecode emitted
}

void Compiler::visitPanicExpr(PanicExpr* expr) {
    expr->value->accept(*this);
    emitByte(Opcode::OP_PANIC);
}

void Compiler::visitRecoverExpr(RecoverExpr* expr) {
    (void)expr;
    emitByte(Opcode::OP_RECOVER);
}

void Compiler::visitChannelExpr(ChannelExpr* expr) {
    if (expr->capacity) {
        expr->capacity->accept(*this);
    } else {
        emitByte(Opcode::OP_CONST_NUMBER);
        emitByte(0);
    }
    emitByte(Opcode::OP_MAKE_CHANNEL);
}

void Compiler::visitSendExpr(SendExpr* expr) {
    expr->channel->accept(*this);
    expr->value->accept(*this);
    emitByte(Opcode::OP_SEND);
}

void Compiler::visitReceiveExpr(ReceiveExpr* expr) {
    expr->channel->accept(*this);
    emitByte(Opcode::OP_RECEIVE);
}

void Compiler::visitMutexExpr(MutexExpr* expr) {
    (void)expr;
    emitByte(Opcode::OP_MUTEX_NEW);
}

void Compiler::visitWaitGroupExpr(WaitGroupExpr* expr) {
    (void)expr;
    emitByte(Opcode::OP_WAITGROUP_NEW);
}

void Compiler::visitGoStmt(GoStmt* stmt) {
    stmt->function->accept(*this);
    int argCount = 0;
    for (const auto& arg : stmt->args) {
        arg->accept(*this);
        argCount++;
    }
    emitBytes(Opcode::OP_GO, static_cast<uint8_t>(argCount));
}

void Compiler::visitSelectStmt(SelectStmt* stmt) {
    // Compile each case body as a closure for VM to pick
    struct CaseInfo {
        bool isSend;
        bool isDefault;
    };
    std::vector<CaseInfo> infos;

    for (int i = 0; i < static_cast<int>(stmt->cases.size()); i++) {
        auto& cs = stmt->cases[i];
        if (cs.isDefault) {
            Compiler bc; bc.beginScope();
            for (const auto& b : cs.body) b->accept(bc);
            bc.emitByte(Opcode::OP_CONST_UNDEFINED); bc.emitReturn(); bc.endScope();
            Function* func = new Function("sel_def", bc.chunk, 0);
            int fi = static_cast<int>(chunk.constants.size());
            chunk.constants.push_back(Value(func));
            emitByte(Opcode::OP_CLOSURE);
            emitByte(static_cast<uint8_t>(fi));
            emitByte(0);
            emitByte(Opcode::OP_CONST_NULL);
            emitByte(Opcode::OP_CONST_FALSE);
            emitByte(Opcode::OP_CONST_NULL);
            infos.push_back({false, true});
        } else {
            Compiler bc; bc.beginScope();
            for (const auto& b : cs.body) b->accept(bc);
            bc.emitByte(Opcode::OP_CONST_UNDEFINED); bc.emitReturn(); bc.endScope();
            Function* func = new Function("sel_case", bc.chunk, 0);
            int fi = static_cast<int>(chunk.constants.size());
            chunk.constants.push_back(Value(func));
            emitByte(Opcode::OP_CLOSURE);
            emitByte(static_cast<uint8_t>(fi));
            emitByte(0);
            // For receive case, cs.channel is a ReceiveExpr; extract the actual channel
            if (!cs.isSend) {
                auto* recvExpr = dynamic_cast<ReceiveExpr*>(cs.channel.get());
                if (recvExpr) {
                    recvExpr->channel->accept(*this);
                } else {
                    cs.channel->accept(*this);
                }
            } else {
                cs.channel->accept(*this);
            }
            emitByte(cs.isSend ? Opcode::OP_CONST_TRUE : Opcode::OP_CONST_FALSE);
            if (cs.isSend) cs.value->accept(*this);
            else emitByte(Opcode::OP_CONST_NULL);
            infos.push_back({cs.isSend, false});
        }
    }

    emitByte(Opcode::OP_SELECT);
    emitByte(static_cast<uint8_t>(infos.size()));
}

void Compiler::pushTypeParams(const std::vector<std::string>& params) {
    std::unordered_map<std::string, Type> bindings;
    for (const auto& param : params) {
        bindings[param] = Type::undefined();
    }
    typeParamBindings.push_back(std::move(bindings));
}

void Compiler::popTypeParams() {
    if (!typeParamBindings.empty()) {
        typeParamBindings.pop_back();
    }
}

std::string Compiler::resolveTypeParam(const std::string& name) {
    for (int i = static_cast<int>(typeParamBindings.size()) - 1; i >= 0; i--) {
        auto it = typeParamBindings[i].find(name);
        if (it != typeParamBindings[i].end()) {
            return name; // Type param name itself is used as placeholder
        }
    }
    return "";
}

std::string Compiler::mangleTypeName(const std::string& base, const std::vector<Type>& args) {
    if (args.empty()) return base;
    std::string mangled = base;
    for (const auto& arg : args) {
        if (!arg.name.has_value()) {
            reportError({{}, 0, 0}, "Generic type argument has no name.");
            return base;
        }
        mangled += "_" + arg.name.value();
    }
    return mangled;
}

std::string Compiler::mangleTypeName(const std::string& base, const std::vector<std::string>& args) {
    if (args.empty()) return base;
    std::string mangled = base;
    for (const auto& arg : args) {
        mangled += "_" + arg;
    }
    return mangled;
}

void Compiler::visitStructDeclStmt(StructDeclStmt* stmt) {
    declareVariable(stmt->name);
    pushTypeParams(stmt->typeParams);
    // Structs are compile-time only, no bytecode emitted
    popTypeParams();
}

void Compiler::visitStructInstantiationExpr(StructInstantiationExpr* expr) {
    std::string structName = expr->name;
    if (!expr->typeArgs.empty()) {
        structName = mangleTypeName(expr->name, expr->typeArgs);
    }
    
    std::vector<int> fieldNameIndices;
    
    for (const auto& field : expr->fields) {
        field.value->accept(*this);
        fieldNameIndices.push_back(chunk.findOrAddStringConstant(field.name));
    }
    
    int nameIndex = chunk.findOrAddStringConstant(structName);
    emitByte(Opcode::OP_NEW_STRUCT);
    emitByte(static_cast<uint8_t>(nameIndex));
    emitByte(static_cast<uint8_t>(fieldNameIndices.size()));
    
    for (int i = static_cast<int>(fieldNameIndices.size()) - 1; i >= 0; i--) {
        emitByte(static_cast<uint8_t>(fieldNameIndices[i]));
    }
}

void Compiler::visitCallExpr(CallExpr* expr) {
    // Check for built-in functions
    if (auto ident = std::dynamic_pointer_cast<IdentifierExpr>(expr->callee)) {
        if (ident->name == "len") {
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emitByte(Opcode::OP_LEN);
            return;
        } else if (ident->name == "make") {
            // make(type, capacity) — first arg is type name
            if (!expr->arguments.empty()) {
                // Get type name from first argument
                std::string typeName;
                if (auto identArg = std::dynamic_pointer_cast<IdentifierExpr>(expr->arguments[0])) {
                    typeName = identArg->name;
                } else if (auto literalArg = std::dynamic_pointer_cast<LiteralExpr>(expr->arguments[0])) {
                    typeName = literalArg->value.toString();
                } else if (std::dynamic_pointer_cast<ChannelExpr>(expr->arguments[0])) {
                    typeName = "channel";
                }
                
                // Push type name as string constant
                int typeIdx = chunk.findOrAddStringConstant(typeName);
                emitByte(Opcode::OP_CONST_STRING);
                emitByte(static_cast<uint8_t>(typeIdx));
                
                // Push remaining args (capacity, length)
                for (size_t i = 1; i < expr->arguments.size(); i++) {
                    expr->arguments[i]->accept(*this);
                }
            }
            emitByte(Opcode::OP_MAKE);
            return;
        } else if (ident->name == "append") {
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emitByte(Opcode::OP_APPEND);
            return;
        } else if (ident->name == "copy") {
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emitByte(Opcode::OP_COPY);
            return;
        } else if (ident->name == "delete") {
            for (const auto& arg : expr->arguments) {
                arg->accept(*this);
            }
            emitByte(Opcode::OP_DELETE);
            return;
        }
    }
    
    // Generic function call with explicit type args
    if (!expr->typeArgs.empty()) {
        // Get callee name for mangling
        std::string calleeName = "__unknown";
        if (auto calleeIdent = std::dynamic_pointer_cast<IdentifierExpr>(expr->callee)) {
            calleeName = calleeIdent->name;
        }
        // Mangle the function name with type args
        std::string mangledName = mangleTypeName(calleeName, expr->typeArgs);
        
        // Push callee (the function)
        expr->callee->accept(*this);
        
        // Push arguments
        int argCount = 0;
        for (const auto& arg : expr->arguments) {
            arg->accept(*this);
            argCount++;
        }
        
        // Push type arg names as string constants (on top of stack, so VM pops them first)
        for (int i = static_cast<int>(expr->typeArgs.size()) - 1; i >= 0; i--) {
            int idx = chunk.findOrAddStringConstant(expr->typeArgs[i].toString());
            emitByte(Opcode::OP_CONST_STRING);
            emitByte(static_cast<uint8_t>(idx));
        }
        
        // OP_CALL_GENERIC: argCount (low byte), typeArgCount (high byte)
        emitByte(Opcode::OP_CALL_GENERIC);
        emitByte(static_cast<uint8_t>(argCount));
        emitByte(static_cast<uint8_t>(expr->typeArgs.size()));
        return;
    }
    
    // Regular function call - push args first (in reverse), then callee
    std::vector<Expr::Ptr> reversedArgs = expr->arguments;
    std::reverse(reversedArgs.begin(), reversedArgs.end());
    for (const auto& arg : reversedArgs) {
        arg->accept(*this);
    }
    expr->callee->accept(*this);
    int argCount = static_cast<int>(expr->arguments.size());
    emitCall(argCount);
}

void Compiler::visitFunctionStmt(FunctionStmt* stmt) {
    // Don't declare as local - functions are globals
    
    Compiler bodyCompiler;
    bodyCompiler.beginScope();
    
    if (!stmt->typeParams.empty()) {
        bodyCompiler.pushTypeParams(stmt->typeParams);
    }
    
    for (const auto& param : stmt->parameters) {
        bodyCompiler.declareVariable(param.first);
    }
    
    for (const auto& bodyStmt : stmt->body) {
        bodyStmt->accept(bodyCompiler);
    }
    bodyCompiler.endScope();
    
    Function* func = new Function(stmt->name, bodyCompiler.chunk, 
                                  static_cast<int>(stmt->parameters.size()));
    
    int funcIndex = static_cast<int>(chunk.constants.size());
    chunk.constants.push_back(Value(func));
    
    emitByte(Opcode::OP_CLOSURE);
    emitByte(static_cast<uint8_t>(funcIndex));
    emitByte(0);
    
   int slot = resolveLocal(stmt->name, false);
    if (slot >= 0) {
        int depth = resolveLocalDepth(stmt->name);
        if (depth == 0) {
            // Global scope - use OP_SET_GLOBAL
            int nameIdx = chunk.findOrAddStringConstant(stmt->name);
            emitByte(Opcode::OP_SET_GLOBAL);
            emitByte(static_cast<uint8_t>(nameIdx));
        } else {
            // Local scope - use OP_SET_LOCAL
            emitSetLocal(slot);
        }
    } else {
        int nameIdx = chunk.findOrAddStringConstant(stmt->name);
        emitByte(Opcode::OP_SET_GLOBAL);
        emitByte(static_cast<uint8_t>(nameIdx));
    }
}

void Compiler::visitTypeAssertExpr(TypeAssertExpr* expr) {
    expr->expression->accept(*this);
    std::string typeName = expr->type.toString();
    int typeIdx = chunk.findOrAddStringConstant(typeName);
    emitByte(Opcode::OP_TYPE_CHECK);
    emitByte(static_cast<uint8_t>(typeIdx));
}

void Compiler::visitRangeStmt(RangeStmt* stmt) {
    int savedLoopDepth = loopDepth;
    enterLoop();
    
    for (auto& var : stmt->variables) {
        declareVariable(var);
        defineVariable(var);
    }
    
    std::string tempName = "__range_iter";
    declareVariable(tempName);
    defineVariable(tempName);
    
    stmt->collection->accept(*this);
    
    int valueSlot = -1;
    int indexSlot = -1;
    if (stmt->variables.size() == 2) {
        indexSlot = resolveLocal(stmt->variables[0], false);
        valueSlot = resolveLocal(stmt->variables[1], false);
    } else if (stmt->variables.size() == 1) {
        valueSlot = resolveLocal(stmt->variables[0], false);
    }
    
    int tempIdx = chunk.findOrAddStringConstant(tempName);
    emitByte(Opcode::OP_SET_GLOBAL);
    emitByte(static_cast<uint8_t>(tempIdx));
    
    int lenIdx = chunk.findOrAddStringConstant("__range_len");
    declareVariable("__range_len");
    defineVariable("__range_len");
    emitByte(Opcode::OP_GET_GLOBAL);
    emitByte(static_cast<uint8_t>(tempIdx));
    emitByte(Opcode::OP_ARRAY_LENGTH);
    emitByte(Opcode::OP_SET_GLOBAL);
    emitByte(static_cast<uint8_t>(lenIdx));
    
    int indexVarIdx = chunk.findOrAddStringConstant("__range_idx");
    declareVariable("__range_idx");
    defineVariable("__range_idx");
    emitConstant(Value(0.0));
    emitByte(Opcode::OP_SET_GLOBAL);
    emitByte(static_cast<uint8_t>(indexVarIdx));
    
    int loopStart = static_cast<int>(chunk.code.size());
    
    int checkPos = static_cast<int>(chunk.code.size());
    emitByte(Opcode::OP_GET_GLOBAL);
    emitByte(static_cast<uint8_t>(indexVarIdx));
    emitByte(Opcode::OP_GET_GLOBAL);
    emitByte(static_cast<uint8_t>(lenIdx));
    emitByte(Opcode::OP_LESS);
    
    int endJump = -1;
    emitJump(Opcode::OP_JUMP_IF_FALSE, endJump);
    
    emitByte(Opcode::OP_GET_GLOBAL);
    emitByte(static_cast<uint8_t>(indexVarIdx));
    emitByte(Opcode::OP_GET_GLOBAL);
    emitByte(static_cast<uint8_t>(tempIdx));
    emitByte(Opcode::OP_ARRAY_AT);
    
    if (valueSlot >= 0) {
        emitByte(Opcode::OP_SET_LOCAL);
        emitByte(static_cast<uint8_t>(valueSlot));
    } else {
        int varIdx = chunk.findOrAddStringConstant(stmt->variables[0]);
        emitByte(Opcode::OP_SET_GLOBAL);
        emitByte(static_cast<uint8_t>(varIdx));
    }
    
    emitByte(Opcode::OP_GET_GLOBAL);
    emitByte(static_cast<uint8_t>(indexVarIdx));
    emitConstant(Value(1.0));
    emitByte(Opcode::OP_ADD);
    emitByte(Opcode::OP_SET_GLOBAL);
    emitByte(static_cast<uint8_t>(indexVarIdx));
    
    stmt->body->accept(*this);
    
    int loopJump = -1;
    emitJump(Opcode::OP_LOOP, loopJump);
    
    for (auto& jumpOff : loopEndJumps) {
        int offset = loopStart - (jumpOff + 2);
        chunk.code[jumpOff] = static_cast<uint8_t>(offset);
        chunk.code[jumpOff + 1] = static_cast<uint8_t>(offset >> 8);
    }
    loopEndJumps.clear();
    
    int endOffset = static_cast<int>(chunk.code.size()) - endJump - 2;
    chunk.code[endJump] = static_cast<uint8_t>(endOffset);
    chunk.code[endJump + 1] = static_cast<uint8_t>(endOffset >> 8);
    
    int loopOffset = (loopJump + 2) - loopStart;
    chunk.code[loopJump] = static_cast<uint8_t>(loopOffset);
    chunk.code[loopJump + 1] = static_cast<uint8_t>(loopOffset >> 8);
    
    loopDepth = savedLoopDepth;
    exitLoop();
}

void Compiler::visitTypeSwitchStmt(TypeSwitchStmt* stmt) {
    int savedLoopDepth = loopDepth;
    enterLoop();
    
    stmt->expression->accept(*this);
    
    std::vector<int> caseJumpOffsets;
    int switchEndJump = -1;
    
    for (size_t i = 0; i < stmt->cases.size(); i++) {
        auto& tc = stmt->cases[i];
        std::string typeName = tc.type.toString();
        int typeIdx = chunk.findOrAddStringConstant(typeName);
        
        emitByte(Opcode::OP_TYPE_CHECK);
        emitByte(static_cast<uint8_t>(typeIdx));
        
        int jumpOffset = -1;
        emitJump(Opcode::OP_JUMP_IF_TRUE, jumpOffset);
        caseJumpOffsets.push_back(jumpOffset);
        
        for (const auto& s : tc.statements) {
            s->accept(*this);
        }
        
        if (i < stmt->cases.size() - 1) {
            int jumpToEnd = -1;
            emitJump(Opcode::OP_JUMP, jumpToEnd);
            caseJumpOffsets.push_back(jumpToEnd);
        }
    }
    
    switchEndJump = static_cast<int>(chunk.code.size());
    
    for (size_t i = 0; i < caseJumpOffsets.size(); i++) {
        int target = caseJumpOffsets[i] + 2;
        if (i + 1 < caseJumpOffsets.size()) {
            target = caseJumpOffsets[i + 1];
        } else {
            target = switchEndJump;
        }
        int offset = target - (caseJumpOffsets[i] + 2);
        chunk.code[caseJumpOffsets[i]] = static_cast<uint8_t>(offset);
        chunk.code[caseJumpOffsets[i] + 1] = static_cast<uint8_t>(offset >> 8);
    }
    
    for (auto& jumpOff : loopEndJumps) {
        int offset = switchEndJump - (jumpOff + 2);
        chunk.code[jumpOff] = static_cast<uint8_t>(offset);
        chunk.code[jumpOff + 1] = static_cast<uint8_t>(offset >> 8);
    }
    loopEndJumps.clear();
    
    loopDepth = savedLoopDepth;
    exitLoop();
}

} // namespace hs
