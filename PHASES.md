# Harvis Script Compiler - Phase Plans

## Phase A: Fix Broken Features ✅ COMPLETE

### A1: Try/Catch/Throw ✅ COMPLETE
**Commit:** 8dbc349
**Files modified:** vm.h, vm.cpp, compiler.cpp, token.cpp

### A2: Select Statement ✅ COMPLETE
**Commit:** 15000ac
**Files modified:** compiler.cpp, parser.cpp, types.cpp, vm.cpp

### A3: Goroutine Race Conditions ✅ COMPLETE
**Commit:** 8dbc349
**Files modified:** vm.cpp, vm.h

### A4: Import/Export ✅ COMPLETE
**Commit:** 7918467
**Files modified:** compiler.cpp, vm.cpp, vm.h

### A5: Classes/Instances/Method Dispatch ✅ COMPLETE
**Commit:** 3520dd3
**Files modified:** compiler.cpp, vm.cpp, vm.h, object.h

---

## Phase B: New Features (Next)

### B1: Error Handling Improvements
- [ ] Stack trace on panic (show file/line for each frame)
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
- `7918467` feat: fix Phase A4 import/export system and stdlib support
- `3520dd3` feat: implement classes/instances/method dispatch (Phase A5)
- `8dbc349` Implement try/catch/throw with full stack unwinding

**Status:** All Phase A features complete. Ready for Phase B.
