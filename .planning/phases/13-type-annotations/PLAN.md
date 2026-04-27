# Phase 13: Type Annotations - PLAN

## Goal
Implement type annotations on variables and functions, optional chaining (`?.`), and nullish coalescing (`??`).

## Acceptance Criteria
1. Variables can have type annotations: `let x: number = 42;`
2. Functions can have typed parameters: `function foo(a: number, b: string)`
3. Functions can have typed return values: `function foo(): number`
4. Generic type annotations work: `let arr: Array<number>`
5. Optional chaining works: `obj?.prop`, `arr?.[0]`, `fn?.()`
6. Nullish coalescing works: `a ?? b`
7. All existing tests still pass

## Tasks

### Task 1: Lexer Changes
- Add `TokenType::DOT_QUESTION` to token.h
- Add `TokenType::NULLISH_COALESCE` to token.h
- Update lexer.cpp to scan `?.` as `DOT_QUESTION`
- Update lexer.cpp to scan `??` as `NULLISH_COALESCE`
- Add token name mapping in token.cpp

### Task 2: AST Changes
- Change `FunctionStmt::returnType` from `std::string` to `Type`
- Add `BinaryOp::NULLISH_COALESCE` to enum
- Add `BinaryOp::OPTIONAL_CHAIN` to enum (for optional call)
- Update AST constructors to use `Type` for return type

### Task 3: Parser Changes
- Change `parseTypeAnnotation()` to return `Type` instead of `std::string`
- Update `parseFunction()` to use `Type` for return type
- Update `parseParameterList()` signature to match
- Add `parseNullishCoalescing()` between `parseLogicalAnd()` and `parseBitwiseOr()`
- Update `parseCall()` to handle optional chaining (`?.`)
- Update `parseType()` to recognize special types

### Task 4: Compiler Changes
- Update `visitFunctionStmt()` to use `Type` return type
- Update `visitDotExpr()` for optional chaining
- Update `visitIndexExpr()` for optional index access
- Update `visitBinaryExpr()` for nullish coalescing
- Update `visitCallExpr()` for optional calls

### Task 5: VM Changes
- Add `OP_NULLISH_COALESCE` opcode
- Add `OP_GET_FIELD_OPTIONAL` opcode
- Add `OP_GET_INDEX_OPTIONAL` opcode
- Add `OP_CALL_OPTIONAL` opcode

### Task 6: Tests
- Create test files for each feature

## Dependencies
- None — builds on existing type system scaffolding

## Risk Assessment
- Low risk: Optional chaining and nullish coalescing are expression-level changes
- Medium risk: Need to handle precedence correctly in parser
- Low risk: No runtime type enforcement needed
