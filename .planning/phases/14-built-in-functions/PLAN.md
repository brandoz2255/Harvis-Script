# Phase 14: Built-in Functions - PLAN

## Goal
Implement built-in functions (`len`, `make`, `append`, `copy`, `delete`), range loops, type assertions, and type switches.

## Acceptance Criteria
1. `len(collection)` returns length for arrays, strings, and maps
2. `make(type, params)` allocates initialized arrays, maps, and channels
3. `append(array, element)` returns new array with element added
4. `copy(dest, src)` copies elements between arrays
5. `delete(map, key)` removes key from map
6. `for x in collection` loop syntax works
7. `for i, x in array` index iteration works
8. `for k, v in map` key-value iteration works
9. `expr as Type` type assertion works
10. `switch expr { case Type: ... }` type switch works
11. All existing tests continue to pass

## Tasks

### Task 1: Lexer Changes
- Check if `range` and `as` keywords already exist in lexer
- Add `TokenType::RANGE` if needed
- Add `TokenType::AS` if needed
- Add keyword mappings

### Task 2: AST Changes
- Add `RangeStmt` class (extends Stmt) with fields: variable(s), collection, body
- Add `TypeAssertExpr` class (extends Expr) with fields: expression, type
- Add `TypeCase` class for type switch cases (type + body)
- Add `TypeSwitchStmt` class (extends Stmt) with fields: expression, cases, default body

### Task 3: Parser Changes
- Add `parseRangeStmt()` method
- Add `parseTypeAssertExpr()` method in expression parsing
- Add `parseTypeSwitchStmt()` method
- Update expression precedence for `as` postfix operator
- Update existing `parseSwitchStmt()` to distinguish value switch from type switch

### Task 4: Compiler Changes
- Add `OP_LEN`, `OP_MAKE`, `OP_APPEND`, `OP_COPY`, `OP_DELETE` opcodes
- Add range loop compilation (iterators for arrays, strings, maps, channels)
- Add type assertion compilation
- Add type switch compilation
- Update `visitCallExpr()` to detect built-in function calls

### Task 5: VM Changes
- Implement `OP_LEN` — return length based on value type
- Implement `OP_MAKE` — allocate based on type argument
- Implement `OP_APPEND` — create new array with appended element
- Implement `OP_COPY` — copy elements between arrays
- Implement `OP_DELETE` — remove key from map
- Implement range iteration bytecode
- Implement type assertion check
- Implement type switch dispatch

### Task 6: Tests
- Create comprehensive test files for each feature
- Verify edge cases (empty collections, type mismatches, etc.)

## Dependencies
- Depends on Phase 13 (Type Annotations) for type syntax support
- Depends on existing array/map/channel runtime from Phase 11

## Risk Assessment
- Medium risk: Range loops require iterator abstraction
- Medium risk: Type switch requires runtime type information
- Low risk: Built-in functions are straightforward stack operations
- Low risk: Type assertion is simple type check
