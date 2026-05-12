# Compiler Project Checkpoint — 2026-04-29

## Status: Phase 4 (Stack VM) — Recursion Bug Blocking

### Completed Phases
- **Phase 1**: Lexer (tokenizer)
- **Phase 2**: Parser (AST builder)
- **Phase 3**: Bytecode Compiler (AST → bytecode)
- **Phase 4**: Stack VM (partially complete — function calls work, recursion broken)

### Current Bug
**Symptom**: Recursive function calls crash with `Runtime Error: Operands must be numbers.`

**Root cause identified**: `OP_CALL` handler stack layout issue. When a function is called inside another function's body (e.g., `recur` calling `recur` recursively), the VM stack accumulates leftover values (comparison results like `false` from `n <= 0` checks). The `OP_CALL` opcode's `peek(argCount)` reads the wrong stack element because garbage is left below the callee.

**Debug output** (from `/tmp/min_rec2.hs`):
```
[DEBUG OP_CALL] argCount=1 stack.size()=5 [0]=[NativeFunction print] [1]=false [2]=false [3]=false [4]=0.000000
Error: Can only call functions.
```
The callee `recur` is being overwritten/corrupted — the stack shows `0.000000` (a number) where the function should be.

### Files Modified
- `src/vm/vm.cpp` — OP_CALL handler patched (saves args before popping, fixed stackStart offset)
- `src/vm/vm.cpp` — OP_RETURN handler patched (removed double-pop of args/callee since OP_CALL already pops them)
- `src/vm/vm.cpp` — Debug logging added to OP_CALL

### Open Questions
1. Why does the stack accumulate `false` values? The `OP_LESS_EQUAL` comparison result isn't being consumed by the jump instruction properly.
2. The jump offsets in OP_JUMP_IF_FALSE and OP_JUMP might be leaving stale values on the stack.
3. Need to check if `if` statement compilation properly pops the condition.

### Build Status
- Compiles successfully with warnings (unused variables in OP_INDEX and OP_MAP_SET handlers)
- Executable at `./build/hs`

### Next Steps
1. Fix the `if`/condition stack handling — ensure condition is popped before jump
2. Test recursion after fix
3. Complete Phase 4, then move to Phase 5 (Standard Library)

### Test Cases
- `/tmp/rec_test.hs` — Simple function call (PASS)
- `/tmp/min_rec.hs` — inc function (PASS)
- `/tmp/min_rec2.hs` — Nested function calls with recursion (FAIL)
- `/tmp/test11.hs` — Full recursion test with fib/factorial (FAIL)

### Opencode IDE Status
- FIXED: Removed incompatible `@ai-sdk/openai-compatible` plugin from config
- Opencode now works natively with the OpenAI-compatible provider
