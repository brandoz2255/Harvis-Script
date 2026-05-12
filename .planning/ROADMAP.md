# Harvis Script - Roadmap

## Current State
- Lexer: Full tokenization with keyword support
- Parser: Variables, functions, control flow, structs, interfaces, defer/panic/recover, channels, go/select
- Compiler: Generates bytecode for most constructs including goroutines/channels
- VM: Stack-based execution with closures, arrays, objects, channels, sync primitives
- Memory: Fixed leak issues (ref counting), local variables separated from stack

## Completed Phases

### Phase 10: Structs & Go Error Handling ✓
- Struct declarations and instantiation
- defer mechanism (LIFO deferred execution)
- panic/recover (Go-style structured error handling)
- interfaces (implicit duck typing)

### Phase 11: Goroutines & Channels ✓
- goroutines (lightweight concurrent execution) — opcodes defined, threading has race conditions
- channels (goroutine communication) — buffered/unbuffered, send/receive working
- select statement — opcodes defined, VM handler stubbed
- sync primitives (mutex, waitgroup) — runtime objects defined, VM handlers implemented

### Phase 12: Generics ✓
- Generic struct declarations with type parameters
- Generic struct instantiation with concrete type arguments
- Generic function declarations with type parameters
- Generic function calls with explicit type arguments
- Monomorphization via mangled type names at compile time
- Multiple type parameters supported

### Phase 13: Type Annotations ✓
- Type annotations on variables and functions
- Type narrowing and inference
- Optional chaining (?.)
- Nullish coalescing (??)

### Phase 14: Built-in Functions ✓
- len(), make(), append(), copy(), delete() — implemented and working
- range loop — implemented using OP_ARRAY_LENGTH + OP_ARRAY_AT (arrays only)
- type assertion (`as` operator) — parser + compiler implemented and working
- type switch — parser + compiler implemented
- reflection basics — not started

### Phase 15: Package System ⚠️ IN PROGRESS
- Package declarations
- Module resolution
- Build tooling
- Standard library basics

## Future Phases (Post-MVP)
- AOT compilation (bytecode serialization to .hso)
- Native code generation (JIT)
- Garbage collection
- Full class system
- Standard library
- Discrete math proof support
