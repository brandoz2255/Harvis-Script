# Phase 13: Type Annotations - SUMMARY

## What Was Implemented

### 1. Lexer Changes (`token.h`, `lexer.cpp`, `token.cpp`)
- Added `TokenType::DOT_QUESTION` for `?.` operator
- Added `TokenType::NULLISH_COALESCE` for `??` operator
- Updated lexer to scan `?.` and `??` as single tokens (not separate DOT+QUESTION/QUESTION+QUESTION)
- Added token name mappings for debugging

### 2. AST Changes (`ast.h`)
- Added `BinaryOp::NULLISH_COALESCE` to the BinaryOp enum
- Changed `FunctionStmt::returnType` from `std::string` to `Type` (the existing Type struct)
- `DotExpr::optional` and `IndexExpr::optional` fields already existed from prior scaffolding

### 3. Parser Changes (`parser.cpp`, `parser.h`)
- `parseTypeAnnotation()` now returns `Type` instead of `std::string`
- Added `parseNullishCoalescing()` in the expression precedence chain (between `parseLogicalAnd()` and `parseBitwiseOr()`)
- Updated `parseCall()` to handle optional chaining (`?.prop`, `?.()`, `?.[index]`)
- Updated `parseFunction()` to use `Type` for return type
- Generic type annotations parse correctly: `Array<number>`

### 4. Compiler Changes (`compiler.cpp`, `opcode.h`)
- Added `OP_GET_FIELD_OPTIONAL` opcode
- Added `OP_GET_INDEX_OPTIONAL` opcode
- Added `OP_NULLISH_COALESCE` opcode
- `visitDotExpr()` handles optional chaining — emits `OP_GET_FIELD_OPTIONAL` when `optional=true`
- `visitIndexExpr()` handles optional index access — emits `OP_GET_INDEX_OPTIONAL` when `optional=true`
- `visitBinaryExpr()` for `NULLISH_COALESCE` compiles both operands then emits `OP_NULLISH_COALESCE`

### 5. VM Changes (`vm.cpp`)
- Added `OP_DUP` opcode (duplicates top of stack — added during debugging, kept for utility)
- Added `OP_GET_FIELD_OPTIONAL` handler — returns `undefined` if receiver is null/undefined
- Added `OP_GET_INDEX_OPTIONAL` handler — returns `undefined` if receiver is null/undefined
- Added `OP_NULLISH_COALESCE` handler — pops left and right, pushes left if not null/undefined, else pushes right

## Bugs Fixed During Implementation

### Bug 1: Double left operand compilation
- **Issue**: `visitBinaryExpr()` compiled `expr->left` twice for `NULLISH_COALESCE` (once at line 184, once at line 205)
- **Fix**: Only compile left operand in the switch case for `NULLISH_COALESCE`, not in the common preamble

### Bug 2: Jump-based nullish coalescing stack corruption
- **Issue**: Initial jump-based implementation had incorrect jump offsets causing "undefined variable" errors
- **Root cause**: Jump offset calculation was wrong (`i + offset` vs `i + 2 + offset`)
- **Fix**: Switched to simple stack-based approach — both operands are compiled, then `OP_NULLISH_COALESCE` opcode handles the logic

### Bug 3: Missing right operand for non-NULLISH_COALESCE binary ops
- **Issue**: When adding `NULLISH_COALESCE` special case, accidentally removed `expr->right->accept(*this)` for all other binary ops
- **Fix**: Restored right operand compilation in the non-NULLISH_COALESCE branch

## Test Results

All nullish coalescing and optional chaining features work correctly:
- Single nullish coalescing: `a ?? 42` where `a = null` → `42`
- Multiple nullish coalescing: `a ?? b ?? 42` → `42` (chained correctly)
- Non-null fallback: `c ?? 99` where `c = 10` → `10`
- Expression on right side: `val ?? (5 + 3)` → `8`
- Optional chaining: `obj?.prop` returns `undefined` when `obj` is null
- Type annotations parse and store type info correctly

## Files Modified

- `/src/lexer/token.h` — Added DOT_QUESTION, NULLISH_COALESCE token types
- `/src/lexer/lexer.cpp` — Added scanning for `?.` and `??`
- `/src/lexer/token.cpp` — Added token name mappings
- `/src/ast/ast.h` — Added BinaryOp::NULLISH_COALESCE, changed FunctionStmt::returnType to Type
- `/src/parser/parser.cpp` — Added parseNullishCoalescing(), updated optional chaining handling
- `/src/parser/parser.h` — Added parseNullishCoalescing() declaration, changed parseTypeAnnotation() return type
- `/src/compiler/opcode.h` — Added OP_GET_FIELD_OPTIONAL, OP_GET_INDEX_OPTIONAL, OP_NULLISH_COALESCE, OP_DUP
- `/src/compiler/compiler.cpp` — Added nullish coalescing and optional field/index bytecode generation
- `/src/vm/vm.cpp` — Added handlers for optional field/index, nullish coalescing, OP_DUP
- `/src/main.cpp` — Updated bytecode printer for new opcodes

## Known Limitations

- Type annotations are parsed but not enforced at compile-time or runtime (gradual typing, syntax only)
- No type inference
- No type narrowing
- Optional chaining doesn't support method invocation with arguments (`obj?.method(arg)`) — only parameterless calls
