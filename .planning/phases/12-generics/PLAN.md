# Phase 12: Generics - PLAN

## Goal
Implement generic type parameters on structs and functions with monomorphization, plus generic collections (Array<T>, Map<K,V>).

## Acceptance Criteria
1. Generic struct declarations with type parameters compile and run
2. Generic struct instantiation with concrete type arguments works
3. Generic function declarations with type parameters work
4. Generic function calls with explicit type arguments work
5. Array<T> type parameter is parsed and used in array literals
6. Multiple type parameters supported (e.g., Pair<K, V>)
7. All existing tests still pass

## Implementation Tasks

### Task 1: AST Changes (ast.h)
- Add `typeParams` field to `StructDeclStmt` (vector of strings)
- Add `typeArgs` field to `StructInstantiationExpr` (vector of Type)
- Add `typeParams` field to `VarDeclStmt` (optional Type for declared type)
- Add `typeParams` field to `FunctionStmt` (vector of strings)

### Task 2: Parser Changes (parser.cpp, parser.h)
- Extend `parseType()` to parse parameterized types: `parseTypeArgs()` helper
- Extend `parseStruct()` to parse optional `<T, U, ...>` type params
- Extend `parseStructInstantiation()` to parse optional `<T, U, ...>` type args
- Extend `parseFunction()` to parse optional `<T>` type params after function name
- Extend `parseVariableDeclaration()` to parse optional type annotations with generics

### Task 3: Type System Changes (types.h, types.cpp)
- `Type::of()` already supports params — use it for generic types
- Add `Type::withParams(name, params)` factory helper
- Ensure `Type::toString()` handles generic types

### Task 4: Compiler Changes (compiler.cpp, compiler.h)
- Track generic type bindings in compiler state (type param → concrete type map)
- `visitStructDeclStmt()`: Store generic struct definition with type params
- `visitStructInstantiationExpr()`: Resolve type args, generate concrete struct name
- `visitFunctionStmt()`: Compile generic functions with type param bindings
- `visitCallExpr()`: Support explicit type arguments in function calls
- Add helper to generate mangled type names for monomorphization

### Task 5: VM Changes (vm.cpp)
- `OP_NEW_STRUCT`: Use concrete (non-generic) struct names
- No VM changes needed — monomorphization is compile-time

### Task 6: Main/Bytecode Printing (main.cpp)
- Update bytecode printer to show generic type args

### Task 7: Tests
- test_generics.hs — basic generic struct
- test_generics_func.hs — generic function
- test_generics_multi.hs — multiple type params
- test_array_generic.hs — generic array type

## Dependencies
- None — self-contained phase
- Builds on existing struct system

## Risk Assessment
- Low risk: Structs already compile to MapObj, monomorphization is purely compile-time
- Parser ambiguity: `<` token already exists, need to distinguish generic `<` from comparison
- Minimal VM impact: No runtime changes needed
