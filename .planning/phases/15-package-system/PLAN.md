# Phase 15: Package System Implementation

## Overview
Add a robust package/module system to the Harvis Script compiler. This enables code reusability, modular development, and the foundation for a standard library.

## Goals
- [ ] Package declarations (`package mymodule`)
- [ ] Import/Export system (`import mymodule`, `export func`)
- [ ] Module resolution (local filesystem relative paths)
- [ ] Build tooling updates to support multi-file compilation
- [ ] Basic standard library integration

## Architecture

### 1. Lexer/Parser Changes (`src/lexer`, `src/parser`)
- Add `IMPORT_KEYWORD` and `EXPORT_KEYWORD` tokens if not present.
- Add `PackageStmt` AST node:
  ```cpp
  struct PackageStmt {
      Token name; // package name
  };
  ```
- Add `ImportStmt` AST node:
  ```cpp
  struct ImportStmt {
      Token alias;      // optional alias (import foo as bar)
      std::string path; // module path "path/to/module"
  };
  ```
- Update parser to handle top-level `package` and `import` statements.
- Ensure `package` must be the very first non-comment statement in a file.

### 2. Compiler Changes (`src/compiler`)
- **Module Resolution:**
  - Implement a `ModuleResolver` class that takes a base path and resolves relative module paths.
  - Cache parsed modules to avoid re-parsing the same file multiple times.
- **Symbol Tables:**
  - Extend the compiler to maintain a global module map.
  - When `import` is encountered, parse the target file, compile it, and merge its exported symbols into the current scope.
- **Export Handling:**
  - Mark functions/variables with `export` as part of the module's public API.
  - Emit `OP_IMPORT_MODULE` to load dependencies at runtime if needed.

### 3. VM Changes (`src/vm`)
- **Module State:**
  - VM needs to handle module initialization order (dependencies first).
- **OP_IMPORT_MODULE:**
  - Load pre-compiled bytecode from a dependency.
  - Bind imported names to the local scope.

### 4. Build Tooling (`Makefile`, `run_tests.sh`)
- Update the build script to accept multiple `.hs` files or a directory.
- Resolve dependencies automatically during compilation.

## Implementation Order
1. Add `package`, `import`, `export` lexer tokens and AST nodes.
2. Implement parser for import/export syntax.
3. Implement `ModuleResolver` in the compiler to find and read files.
4. Update compiler to parse dependencies before the main file.
5. Implement `OP_IMPORT_MODULE` in VM to bind foreign symbols.
6. Create `stdlib/core.hs` as the first standard library module.
7. Write tests for multi-file imports and exports.

## Key Design Decisions
- **Static Resolution:** Modules are resolved at compile-time (not runtime) for performance and safety.
- **Single Package Per File:** Each `.hs` file belongs to exactly one package.
- **Path-based Imports:** Imports use relative filesystem paths (`import "./math"` or `import "stdlib/math"`).
- **Aliasing:** Allow `import math as m` to prevent naming collisions.
