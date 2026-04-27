# Phase 14: Built-in Functions - DESIGN

## Overview
Add built-in runtime functions and range loop syntax to Harvis Script. These are special constructs recognized by the compiler and implemented in the VM.

## Design Decisions

### 1. Built-in Functions as Special Calls
- Built-in functions are parsed as regular function calls but compiled specially
- Compiler recognizes built-in names and emits dedicated opcodes or special bytecode
- VM handlers execute the built-in logic with stack-based arguments

### 2. Range Loop Syntax
- `for x in collection { ... }` — iteration over arrays, strings, maps, channels
- `for i, x in array { ... }` — index and value iteration
- `for k, v in map { ... }` — key and value iteration
- `for ch in channel { ... }` — receive from channel until closed
- Compiles to equivalent bytecode using `range` opcodes

### 3. Type Assertion
- `expr as Type` — runtime type check with cast
- Returns value if type matches, otherwise panics
- Syntax: postfix operator with type annotation

### 4. Type Switch
- `switch expr { case Type: ... default: ... }` — dispatch on runtime type
- Different from existing switch (which compares values)
- Checks dynamic type of expression against case types

## Implementation Plan

### Task 1: Lexer Changes
- Add `TokenType::RANGE` for `range` keyword (if not already exists)
- Add `TokenType::AS` for `as` keyword in type assertions
- Existing keywords: `len`, `make`, `append`, `copy`, `delete` are identifiers

### Task 2: AST Changes
- Add `RangeStmt` AST node for range loops
- Add `TypeAssertExpr` AST node for type assertions (`expr as Type`)
- Add `TypeSwitchStmt` AST node for type switches
- Extend existing `SwitchStmt` or create new type-specific variant

### Task 3: Parser Changes
- Add `parseRangeStmt()` for range loop parsing
- Add `parseTypeAssertExpr()` for type assertion parsing
- Add `parseTypeSwitchStmt()` for type switch parsing
- Update expression precedence to handle `as` postfix operator

### Task 4: Compiler Changes
- Compile `len()` call to `OP_LEN` opcode
- Compile `make()` call to `OP_MAKE` opcode
- Compile `append()` call to `OP_APPEND` opcode
- Compile `copy()` call to `OP_COPY` opcode
- Compile `delete()` call to `OP_DELETE` opcode
- Compile `RangeStmt` to iteration bytecode
- Compile `TypeAssertExpr` to type check opcode
- Compile `TypeSwitchStmt` to type dispatch bytecode

### Task 5: VM Changes
- Implement `OP_LEN` — push length of array/string/map
- Implement `OP_MAKE` — allocate array/map/channel with parameters
- Implement `OP_APPEND` — create new array with element appended
- Implement `OP_COPY` — copy elements between arrays
- Implement `OP_DELETE` — remove key from map
- Implement range iteration opcodes
- Implement type assertion opcode
- Implement type switch dispatch

### Task 6: Tests
- test_builtins_len.hs — len() for arrays, strings, maps
- test_builtins_make.hs — make() for arrays, maps, channels
- test_builtins_append.hs — append()
- test_builtins_copy.hs — copy()
- test_builtins_delete.hs — delete()
- test_range_loops.hs — range loops for arrays, strings, maps, channels
- test_type_assertion.hs — type assertions
- test_type_switch.hs — type switches
