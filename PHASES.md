# Harvis Script Compiler - Phase Plans

## Phase A: Fix Broken Features (In Progress)

### A1: Try/Catch/Throw ✅ COMPLETE
**Status:** Implemented and committed (8dbc349)

**Changes:**
- `vm.h`: Added `TryFrame` struct, `tryStack` vector
- `vm.cpp`: OP_TRY handler marks try block, OP_CATCH jumps to handler, OP_THROW walks tryStack for unwinding
- `compiler.cpp`: `visitTryStmt` generates OP_TRY, jump to catch, jump over catch
- `compiler.cpp`: `visitThrowStmt` emits OP_THROW
- `token.cpp`: Added try/catch/throw/finally to keyword map (was defined in token.h but missing from map)

**Tests passing:**
- Basic catch, no-error skip, nested try/catch, cross-function unwinding, uncaught throw

**Files modified:** `src/compiler/compiler.cpp`, `src/vm/vm.cpp`, `src/vm/vm.h`, `src/lexer/token.cpp`

---

### A2: Select Statement ✅ COMPLETE
**Status:** Implemented, 3 bugs fixed

**Bugs fixed:**
1. **Value ref counting** (`src/core/types.cpp`): `Value(RuntimeObject*)` constructor now calls `obj->retain()`. Previously did nothing, causing Closures to be deleted when Values went out of scope.
2. **Select receive case compilation** (`src/compiler/compiler.cpp`): For `case <-ch:`, `cs.channel` is a `ReceiveExpr`. Now extracts `recvExpr->channel` instead of compiling the full receive expression (which would execute OP_RECEIVE at setup time).
3. **Parser semicolon tolerance** (`src/parser/parser.cpp`): Made semicolon optional in `parseExpressionStatement()`. Select case bodies use `}`, `case`, `default` as delimiters.

**Tests passing:**
- `select { default: println("d"); }` → prints "d"
- `select { case <-ch: println("received") }` → receives from channel, prints message
- Multiple cases, mixed send/receive cases

**Files modified:** `src/compiler/compiler.cpp`, `src/parser/parser.cpp`, `src/core/types.cpp`, `src/vm/vm.cpp`

---

### A3: Goroutine Race Conditions ✅ COMPLETE
**Status:** Fixed

**Problem:** 340-line inline interpreter inside `OP_GO` with race conditions on shared stack/callStack.

**Solution:**
- Replaced entire inline interpreter with `std::thread` that calls shared `interpret()` method
- Added `std::recursive_mutex vmMutex` to `VM` class
- `interpret()` acquires mutex at start of each opcode iteration
- Uses `recursive_mutex` so `interpret()` can be called recursively from `executeDeferred()`

**Files modified:** `src/vm/vm.cpp` (~340 lines removed, ~10 lines added), `src/vm/vm.h`

---

### A4: Import/Export ⚠️ PARTIALLY WORKING
**Status:** Basic import works, export needs work

**Implemented:**
- `vm.h`: Added `modules` map, `loadModule()` method
- `vm.cpp`: `loadModule()` loads .hs file, compiles it, copies globals to MapObj exports
- `vm.cpp`: OP_IMPORT calls `loadModule()`, OP_EXPORT marks values
- `compiler.cpp`: Auto-generates variable name from module filename for shorthand import (`import "math.hs"` → `math`)
- `compiler.cpp`: `visitFunctionStmt` uses `OP_SET_GLOBAL` directly instead of local scope
- `src/core/types.h`: Added `Type::module()` static method

**Tests passing:**
- `import "modules/math.hs"` creates `math` variable with exported functions
- `math.add(5, 3)` → 8.0, `math.subtract(10, 4)` → 6.0

**Remaining work for A4:**
- [ ] Fix export to properly register functions (currently exports MapObj but function values are stored as strings, not Closures)
- [ ] Support `from` clause: `import { add } from "math.hs"`
- [ ] Support namespace import: `import * as math from "math.hs"`
- [ ] Handle module caching (don't recompile same module twice)
- [ ] Standard library directory support (resolve relative paths + stdlib path)

**Files modified:** `src/vm/vm.cpp`, `src/vm/vm.h`, `src/compiler/compiler.cpp`, `src/core/types.h`

---

### A5: Classes/Instances/Method Dispatch ❌ NOT STARTED
**Status:** Not implemented

**Current state:**
- `Instance` class exists in `object.h` but `OP_NEW_CLASS` creates a bare `Function*` instead
- No method dispatch mechanism
- No class instantiation at runtime

**Plan:**
- [ ] Fix `OP_NEW_CLASS` to create proper class object (MapObj with methods)
- [ ] Add `OP_NEW_INSTANCE` to create instance from class, initialize fields
- [ ] Add `OP_METHOD` to bind methods to class (instance + function)
- [ ] Implement method dispatch via `OP_CALL` when callee is BoundMethod
- [ ] Support `this` keyword in methods (passed as first argument or via BoundMethod)
- [ ] Support inheritance via `OP_INHERIT` (set up base prototype chain)

**Files to modify:** `src/vm/vm.cpp`, `src/vm/object.h`, `src/compiler/compiler.cpp`

---

## Phase B: New Features (Future)

### B1: Error Handling Improvements
- [ ] Add stack trace on panic (show file/line for each frame)
- [ ] Support custom error types
- [ ] Add `errors.Is()` and `errors.As()` equivalent

### B2: Standard Library
- [ ] Implement stdlib directory structure
- [ ] Add `math`, `strings`, `fmt` packages
- [ ] Support `import "math"` resolving to stdlib path

### B3: Performance Optimizations
- [ ] JIT compilation for hot paths
- [ ] Bytecode caching (compile once, cache binary)
- [ ] Inline caching for property access

---

## Current Git State

**Branch:** master
**Commits:**
- `8dbc349` Implement try/catch/throw with full stack unwinding
- `15000ac` Fix make(channel, 3) - handle ChannelExpr as valid type argument
- `bc712d3` Initial commit: Harvis Script bytecode compiler

**Unstaged changes:**
- `src/compiler/compiler.cpp` - Function export fix, select receive fix, import variable naming
- `src/core/types.cpp` - Value ref counting fix
- `src/parser/parser.cpp` - Optional semicolon in expression statements
- `src/vm/vm.cpp` - Goroutine fix, select fix, import/export implementation
- `src/vm/vm.h` - Module system, goroutine mutex

**Next action:** Decide whether to commit current changes before starting A5.
