# Harvis Script - Project State

## Last Update
Date: 2026-04-20
Phase: 12 (Generics) - COMPLETE

## Key Decisions
- Go-style performance (goroutines, channels) — high-speed execution
- TypeScript-like features (interfaces, generics, types)
- Classes preserved alongside structs — not a rewrite, incremental enhancement
- Go-like syntax for error handling (defer, panic, recover)
- Build something real with this language when done

## Completed Phases
- Phase 10: Structs & Go Error Handling ✓
- Phase 11: Goroutines & Channels ✓
- Phase 11.5: Gap Fixes ✓
  - Break/Continue: Implemented with loop depth tracking
  - Switch: Implemented with bytecode generation
  - Bitwise operators: AND, OR, XOR, NOT, SHIFT_LEFT, SHIFT_RIGHT
  - Class system: Parser now builds ClassStmt with methods/fields
  - Local variable scoping: Fixed using shared constant pool
  - Expression statements: Fixed to skip OP_POP for assignments
- Phase 12: Generics ✓
  - Generic struct declarations: `struct Box<T> { value: T }`
  - Generic struct instantiation: `new Box<number> { value: 42 }`
  - Generic function declarations: `function identity<T>(x: T): T`
  - Generic function calls: `identity<number>(100)`
  - Multiple type parameters: `struct Pair<A, B> { first: A; second: B }`
  - Monomorphization via mangled type names (Box_number, Pair_number_string)
  - Type parameter bindings in compiler for generic functions
  - Fixed OP_CALL to initialize locals from stack arguments

## Pending Phases
- Phase 13: Type Annotations
- Phase 14: Built-in Functions
- Phase 15: Package System

## Known Issues
- Goroutine threading has race conditions (shared VM state)
- Class system still non-functional (VM needs Instance objects)
- Switch default case not fully implemented
- Import/Export are NOPs
- No garbage collection
- No standard library beyond print/println

## Recent Fixes
- Fixed OP_DEFERRED_RETURN (now uses interpret() with proper return handling)
- Fixed OP_GET_FIELD/OP_SET_FIELD double ip increment
- Fixed local variable storage (separate locals array)
- Fixed struct field naming (OP_NEW_STRUCT reads field names from constant pool)
- Fixed OP_SEND to push undefined result
- Fixed OP_IS_INSTANCE/OP_INVOKE double ip increment
- Added bitwise operator opcodes and VM handlers
- Added break/continue with loop depth tracking
- Added switch statement bytecode generation
- Fixed class parser to build ClassStmt with methods/fields
- Fixed local variable scoping (shared constant pool, proper opcode selection)
- Fixed expression statements to skip OP_POP for assignments
- Fixed OP_CALL to initialize locals from stack arguments (functions now work)
- Added generic type parameter support (structs and functions)
- Fixed parser to handle generic type args in function calls
- Fixed bytecode printer to show newlines between instructions
