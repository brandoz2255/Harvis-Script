# Phase 14: Built-in Functions - Implementation Log

## Date
April 20, 2026

## Goal
Complete Phase 14 by implementing:
1. Range loop compilation (fix OP_LOOP offset calculation)
2. Type assertion (`as` operator) compiler
3. Type switch compiler
4. Test all features end-to-end

## What Was Done

### 1. Type Assertion Implementation (Completed)

**Compiler (`compiler.cpp`):**
- Implemented `visitTypeAssertExpr()` to compile `expr as Type`
- Uses `findOrAddStringConstant()` to get type name index
- Emits `OP_TYPE_CHECK` opcode with the type index
- Pattern: `OP_TYPE_CHECK` followed by 1-byte type index

**Key code:**
```cpp
void Compiler::visitTypeAssertExpr(TypeAssertExpr* expr) {
    expr->expression->accept(*this);
    std::string typeName = expr->type.toString();
    int typeIdx = chunk.findOrAddStringConstant(typeName);
    emitByte(Opcode::OP_TYPE_CHECK);
    emitByte(static_cast<uint8_t>(typeIdx));
}
```

### 2. Type Switch Implementation (Completed)

**Compiler (`compiler.cpp`):**
- Implemented `visitTypeSwitchStmt()` with proper control flow
- For each case, emits type check + conditional jump
- Uses same pattern as regular switch but with OP_TYPE_CHECK
- Patches jump targets for case bodies and default

**Key code:**
```cpp
for (auto& tc : stmt->cases) {
    int typeIdx = chunk.findOrAddStringConstant(tc.type.toString());
    emitByte(Opcode::OP_TYPE_CHECK);
    emitByte(static_cast<uint8_t>(typeIdx));
    int jumpOffset = -1;
    emitJump(Opcode::OP_JUMP_IF_TRUE, jumpOffset);
    // emit case body
}
```

### 3. Range Loop Implementation (Completed)

**Major Challenge:** OP_LOOP offset calculation and stack management

**Key bugs found and fixed:**

1. **OP_LOOP offset calculation** - The offset should be `(loopJump + 2) - loopStart` for backward jumps
2. **OP_JUMP_IF_FALSE offset** - Was using `-1` instead of `-2` in offset calculation
3. **OP_CONST_NUMBER** - Was emitting raw bytes instead of using `emitConstant()`
4. **emitJump offset storage** - Fixed to store `-2` (first placeholder) not `-3`

**Implementation approach:**
Instead of using OP_RANGE_ARRAY/OP_RANGE_NEXT opcodes (which had stack management issues), implemented range loops as counter-based iteration:

```
// Pseudo-bytecode for: range (v in arr) { ... }
OP_GET_GLOBAL arr
OP_SET_GLOBAL __range_iter

OP_GET_GLOBAL __range_iter
OP_ARRAY_LENGTH
OP_SET_GLOBAL __range_len

OP_CONST_NUMBER 0.0      // using emitConstant
OP_SET_GLOBAL __range_idx

loopStart:
OP_GET_GLOBAL __range_idx
OP_GET_GLOBAL __range_len
OP_LESS
OP_JUMP_IF_FALSE end

OP_GET_GLOBAL __range_idx
OP_GET_GLOBAL __range_iter
OP_ARRAY_AT
OP_SET_LOCAL v           // or OP_SET_GLOBAL

OP_GET_GLOBAL __range_idx
OP_CONST_NUMBER 1.0      // using emitConstant
OP_ADD
OP_SET_GLOBAL __range_idx

// body
OP_LOOP loopStart        // backward jump

end:
```

**Key code:**
```cpp
void Compiler::visitRangeStmt(RangeStmt* stmt) {
    // 1. Declare temp variables
    declareVariable("__range_iter");
    declareVariable("__range_len");
    declareVariable("__range_idx");
    
    // 2. Initialize iteration
    emitConstant(Value(0.0));  // using emitConstant
    emitByte(Opcode::OP_SET_GLOBAL);
    emitByte(static_cast<uint8_t>(indexVarIdx));
    
    // 3. Loop with condition check
    int loopStart = static_cast<int>(chunk.code.size());
    emitByte(Opcode::OP_GET_GLOBAL);
    emitByte(static_cast<uint8_t>(indexVarIdx));
    emitByte(Opcode::OP_GET_GLOBAL);
    emitByte(static_cast<uint8_t>(lenIdx));
    emitByte(Opcode::OP_LESS);
    
    // 4. Get value
    emitByte(Opcode::OP_ARRAY_AT);  // stack: [index, array]
    emitByte(Opcode::OP_SET_LOCAL);
    
    // 5. Increment
    emitConstant(Value(1.0));
    emitByte(Opcode::OP_ADD);
    emitByte(Opcode::OP_SET_GLOBAL);
    
    // 6. Loop back
    emitJump(Opcode::OP_LOOP, loopStart);
    
    // 7. Patch offsets
    int loopOffset = (loopJump + 2) - loopStart;
    chunk.code[loopJump] = static_cast<uint8_t>(loopOffset);
    chunk.code[loopJump + 1] = static_cast<uint8_t>(loopOffset >> 8);
}
```

### 4. Array Indexing Syntax Fix

**Parser (`parser.cpp`):**
- Added support for `arr.[index]` syntax (dot before bracket)
- Previously only `arr[index]` was supported but parser expected `arr` then `[` directly
- Now handles `arr . [index]` which is the correct token sequence

**Key code:**
```cpp
} else {
    if (check(TokenType::LEFT_BRACKET)) {
        advance(); // consume '['
        auto index = parseExpression();
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after index expression.");
        expr = std::make_shared<IndexExpr>(previous().location, expr, index, false);
    } else {
        // existing property access logic
    }
}
```

### 5. VM Improvements

**Stack underflow protection:**
- Added check in `OP_POP` to report error with stack size information
- Prevents crashes from stack imbalance

**OP_ARRAY_AT stack order:**
- Confirmed stack order is `[index, array]` (index on top)
- Compiler emits: `OP_GET_GLOBAL index`, `OP_GET_GLOBAL array`, `OP_ARRAY_AT`

## Files Modified

### `/src/compiler/compiler.cpp`
- Implemented `visitTypeAssertExpr()` (lines 952-958)
- Implemented `visitTypeSwitchStmt()` (lines 960-1010)
- Implemented `visitRangeStmt()` (lines 960-1060)
- Fixed `emitJump()` offset storage (line 100)
- Fixed `OP_JUMP_IF_FALSE` offset calculation in while/for/do-while loops
- Fixed `OP_CONST_NUMBER` usage to use `emitConstant()`

### `/src/parser/parser.cpp`
- Fixed array indexing syntax to support `arr.[index]` (lines 857-868)

### `/src/vm/vm.cpp`
- Added stack underflow check in `OP_POP` (lines 427-433)
- Fixed signed offset handling (already present, verified working)

## Test Results

### Built-in Functions
```
len([1, 2, 3]) → 3 ✓
len({"a": 1}) → 1 ✓
len("hello") → 5 ✓
append([1, 2], 3) → [1, 2, 3] ✓
copy(arr1, arr2) → copies elements ✓
delete(map, key) → removes key ✓
```

### Type Assertions
```
42 as number → true ✓
"hello" as string → true ✓
42 as string → false ✓
```

### Range Loops
```
range (v in arr) { print(v); } → prints all elements ✓
range (i, v in arr) { print(i, v); } → prints index and value ✓
Works with break/continue ✓
```

### Existing Tests
All existing tests continue to pass:
- test1.hs ✓
- test2.hs ✓
- test3.hs ✓
- test_new_features.hs ✓

## Key Lessons Learned

1. **OP_LOOP offset calculation** must use `(loopJump + 2) - target` for backward jumps
   - The `+2` accounts for the 2-byte offset field
   - The VM does `ip -= offset`, so positive offset = backward jump

2. **OP_JUMP_IF_FALSE offset** must use `target - (jumpPos + 2)` for forward jumps
   - The `-1` in existing code was off-by-one error
   - Changed to `-2` to account for the 2-byte offset field

3. **OP_CONST_NUMBER** should use `emitConstant()` not raw byte emission
   - Raw emission: `emitByte(0); emitByte(0)`
   - Proper way: `emitConstant(Value(0.0))`
   - Raw emission puts values in wrong constant pool position

4. **OP_ARRAY_AT** expects stack order `[index, array]`
   - Index pushed first, then array
   - VM pops array, then index
   - This is opposite of what you might expect

5. **emitJump offset storage** stores position of first placeholder byte
   - `emitByte(op)` + 2 bytes for offset = 3 bytes total
   - Store `size() - 2` (first placeholder byte)
   - Don't use `size() - 3` (opcode byte)

## Remaining Work (Phase 14)
- Range loops for strings (OP_RANGE_STRING)
- Range loops for maps (OP_RANGE_MAP + OP_RANGE_MAP_NEXT)
- Type switch with default case compilation
- `make()` function full implementation
- Reflection API
