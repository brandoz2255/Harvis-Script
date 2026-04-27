# Phase 13: Type Annotations - DESIGN

## Overview
Add proper type annotations on variables and functions, plus optional chaining (`?.`) and nullish coalescing (`??`).

## Design Decisions

### 1. Type Annotations (syntax only, no enforcement)
- Variables: `let x: number = 42`
- Functions: `function foo(a: number): number`
- Parameters: `function foo(a: number, b: string)`
- Generic types: `let arr: Array<number>`
- No runtime checks — types are compile-time only (gradual typing)

### 2. Optional Chaining (`?.`)
- `obj?.prop` — returns `undefined` if `obj` is null/undefined
- `arr?.[index]` — returns `undefined` if `arr` is null/undefined
- `func?.()` — calls if `func` is callable, otherwise returns `undefined`
- Chained: `obj?.a?.b?.c` — short-circuits on first null/undefined

### 3. Nullish Coalescing (`??`)
- `a ?? b` — returns `a` if `a` is not null/undefined, otherwise `b`
- Similar to `||` but checks nullish instead of falsy
- Precedence: between `||` and `&&`

### 4. Type System
- Types are compile-time only — no runtime overhead
- Type information stored in AST and used for error checking (future phase)
- `Type::toString()` already handles generics

## Implementation Plan

### Task 1: Lexer Changes (lexer.cpp, token.h)
- Add `TokenType::DOT_QUESTION` (for `?.`)
- Add `TokenType::NULLISH_COALESCE` (for `??`)
- Update lexer to scan `?.` and `??` as single tokens

### Task 2: AST Changes (ast.h)
- Change `FunctionStmt::returnType` from `std::string` to `Type`
- Add `BinaryOp::NULLISH_COALESCE`
- Add `BinaryOp::OPTIONAL_CHAIN` (if needed)
- `DotExpr::optional` and `IndexExpr::optional` already exist

### Task 3: Parser Changes (parser.cpp, parser.h)
- Change `parseTypeAnnotation()` to return `Type` instead of `std::string`
- Update `parseParameterList()` to handle `Type` return
- Update `parseFunction()` to use `Type` for return type
- Add `parseNullishCoalescing()` in expression precedence chain
- Update `parseCall()` to handle optional chaining (`?.`)
- Update `parseType()` to handle special types (`undefined`, `null`, etc.)

### Task 4: Compiler Changes (compiler.cpp, compiler.h)
- Update `visitVarDeclStmt()` to emit type annotation (for future use)
- Update `visitFunctionStmt()` to use `Type` for return type
- Implement `visitDotExpr()` for optional chaining
- Implement `visitBinaryExpr()` for nullish coalescing

### Task 5: VM Changes (vm.cpp)
- Add `OP_NULLISH_COALESCE` opcode handler (jump-based)
- Add `OP_GET_FIELD_OPTIONAL` for optional field access
- Add `OP_GET_INDEX_OPTIONAL` for optional index access

### Task 6: Tests
- test_type_annotations.hs — basic type annotations
- test_optional_chaining.hs — optional chaining
- test_nullish_coalescing.hs — nullish coalescing
