---
phase: 10-structs
plan: 00
type: overview
---

## Phase 10: Structs & Go Error Handling

### Goal
Add Go-style features + TypeScript-like type system to Harvis Script without rewriting existing code.

### Features
| Feature | Status | Plan |
|---------|--------|------|
| Struct declarations | Not started | Plan 01 (opcodes) + Plan 02 (parser) + Plan 03 (compiler) + Plan 04 (VM) |
| Struct instantiation | Not started | Plan 01 + Plan 02 + Plan 03 + Plan 04 |
| Defer statements | Not started | Plan 01 + Plan 02 + Plan 03 + Plan 04 |
| Panic/Recover | Not started | Plan 01 + Plan 02 + Plan 03 + Plan 04 |
| Interfaces | Not started | Plan 01 + Plan 02 + Plan 03 + Plan 04 |

### Dependency Chain
```
Plan 01 (opcodes + AST + lexer)
    ↓
Plan 02 (parser)  ←  Plan 03 (compiler)
    ↓                    ↓
Plan 04 (VM runtime)  ←  Plan 03 + Plan 01
```

### Execution Order
1. Plan 01: Foundation (opcodes, AST nodes, lexer keywords)
2. Plan 02: Parser (parse struct/defer/panic/recover/interface syntax)
3. Plan 03: Compiler (emit correct bytecode)
4. Plan 04: VM (execute new opcodes)

### Syntax Target
```
// Structs (TypeScript-style)
struct Point {
    x: number;
    y: number;
}

let p = new Point{x: 1, y: 2};
print(p.x);

// Defer (Go-style)
defer log("done");

// Panic/Recover (Go-style)
defer print("cleanup");
panic("something went wrong");

// Interfaces (TypeScript-style)
interface Printable {
    print(): string;
}
```
