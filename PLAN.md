# Phase D: Generics Implementation

## Overview
Add generics to the Harvis Script language. This enables:
- Generic functions: `func sort<T>(arr []T) []T`
- Generic classes: `class Stack<T> { items: []T }`
- Type inference for generic type parameters

## Architecture

### 1. Lexer Changes (src/lexer/lexer.cpp/h)
- Add IDENTIFIER type for type parameters (T, U, K, V, etc.)
- No new tokens needed — type params are just identifiers in angle brackets

### 2. Parser Changes (src/parser/parser.cpp/h)
Add new AST nodes:

**TypeParamDecl** — represents a single type parameter in angle brackets
```
struct TypeParamDecl {
    Token name;  // T, U, K, V
};
```

**GenericDeclaration** — marks a function or class as generic
```
struct GenericDeclaration {
    std::vector<TypeParamDecl> params;
};
```

Update existing structures:
- FunctionDecl gets `std::vector<TypeParamDecl> typeParams`
- ClassStmt gets `std::vector<TypeParamDecl> typeParams`
- CallExpr gets `std::vector<Value> typeArgs` (for explicit type args like `sort<int>`)
- NewExpr gets `std::vector<Value> typeArgs`

Parse syntax:
- `func sort<T>(arr []T) []T` → parse angle brackets before parameters
- `class Stack<T> { items: []T }` → parse angle brackets before brace
- `sort<int>(arr)` → parse angle brackets after function name in call
- `Stack<String>()` → parse angle brackets after class name

### 3. AST Changes (src/ast/ast.cpp/h)
Add new classes to the visitor hierarchy.
Update existing classes with typeParams fields.
Implement accept() for new nodes.

### 4. Compiler Changes (src/compiler/compiler.cpp/h)
- Emit new opcodes for generic instantiation
- Track type parameters in the function/class scope
- Generate type checks at runtime
- Handle type argument deduction (when not explicitly provided)

New opcodes:
- `OP_NEW_CLASS_GENERIC` — instantiate a generic class with type args
- `OP_CALL_GENERIC` — call a generic function with type args
- `OP_TYPE_CHECK` — runtime type check for generic constraints

### 5. VM Changes (src/vm/vm.cpp/h)
- Extend the Value system to carry type information for generics
- Add runtime type checking for generic instantiations
- Track generic type parameters during function calls
- Handle type erasure or runtime type storage (we'll go with runtime type storage for simplicity)

### 6. Tests (test_generics/)
Create comprehensive tests:
- test_generics/01_basic_generic_func.hs — generic function with single type param
- test_generics/02_generic_class.hs — generic class instantiation
- test_generics/03_multiple_type_params.hs — multiple type parameters
- test_generics/04_type_inference.hs — type inference when args not provided
- test_generics/05_generic_stack.hs — real world example: generic Stack class

## Implementation Order
1. Lexer/parser AST nodes (no code changes, just planning)
2. Parser updates — add angle bracket parsing
3. Compiler updates — emit generic opcodes
4. VM updates — handle generic opcodes
5. Tests — verify everything works
6. Fix bugs iteratively

## Key Design Decisions
- Use runtime type storage (not compile-time erasure) for simplicity
- Type parameters use single uppercase letters: T, U, K, V
- Implicit type inference when type args not provided
- No type constraints yet (will add in Phase E)
