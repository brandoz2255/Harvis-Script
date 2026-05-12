# Phase D: Generics Implementation Plan

## Goal
Add full generics support to Harvis Script: generic functions, generic classes, and runtime type instantiation.

## 1. Lexer
- No new tokens needed. `<` and `>` already exist. Type parameters (`T`, `U`, `K`, `V`) are just `IDENTIFIER`s.

## 2. Parser & AST
- **New AST Nodes:**
  - `TypeParamDecl` (name: Token)
  - `GenericDeclaration` (params: vector<TypeParamDecl>)
- **Update Existing Nodes:**
  - `FunctionDecl` & `ClassStmt` add `typeParams` field.
  - `CallExpr` & `NewExpr` add `typeArgs` field.
- **Parsing Logic:**
  - Detect `<` after function/class name -> parse type parameter list.
  - Detect `<` after function/class in a call/new expression -> parse explicit type arguments.

## 3. Bytecode Compiler
- **New Opcodes (already in `opcode.h`):**
  - `OP_NEW_CLASS_GENERIC`
  - `OP_CALL_GENERIC`
  - `OP_TYPE_CHECK`
- **Compiler Logic:**
  - Emit type argument counts alongside function/class indices.
  - Handle type inference if `typeArgs` are empty (deduce from argument types).

## 4. VM Execution
- **Value System:**
  - Ensure `Value` can carry runtime type metadata for generics.
- **Opcodes Implementation:**
  - `OP_NEW_CLASS_GENERIC`: Instantiate a class with provided type args, create new class object.
  - `OP_CALL_GENERIC`: Call function with type args, push to generic cache if needed.
  - `OP_TYPE_CHECK`: Verify runtime type matches generic constraint.
- **Caching:**
  - Use `genericCache` (mangled name -> instantiated function) to avoid re-instantiating the same generic combination.

## 5. Tests (`test_generics/`)
- `01_basic_generic_func.hs`: `func identity<T>(val T) T { return val; }`
- `02_generic_class.hs`: `class Box<T> { value: T }`
- `03_multiple_params.hs`: `func pair<T, U>(a T, b U) { ... }`
- `04_type_inference.hs`: `identity(42)` (infers T=int)
- `05_generic_stack.hs`: Full stack implementation using generics.

## Execution Order
1. [ ] AST & Parser updates for `<T>` syntax.
2. [ ] Compiler updates to emit generic opcodes.
3. [ ] VM implementation of generic opcodes.
4. [ ] Test 1 & 2 (Basic Func & Class).
5. [ ] Test 3, 4, 5 (Inference & Complex Cases).
