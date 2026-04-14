# Harvis Script

A bytecode-compiled scripting language written in **C++17**, inspired by TypeScript/Go with **gradual typing**.

## Quick Start

```bash
# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run a script
./hs script.hs

# REPL mode
./hs
```

## Architecture

```
Source Code → Lexer → AST → Bytecode → Stack VM → Execution
```

### Pipeline Overview

1. **Lexer** (1,579 lines) - Tokenizes source code into tokens
2. **Parser** (900 lines) - Builds AST from tokens using precedence climbing
3. **Compiler** (800 lines) - Translates AST to bytecode with symbol tables
4. **VM** (650 lines) - Executes bytecode on a stack-based virtual machine

## Project Structure

```
compiler/
├── CMakeLists.txt              # Build configuration
├── src/
│   ├── core/                   # Core infrastructure
│   │   ├── types.h/.cpp        # Runtime values, types, scopes
│   │   ├── error.h             # Error handling, Result<T>
│   │   └── scanner.h           # Character buffer for scanning
│   │
│   ├── lexer/                  # Phase 1: Lexical Analysis
│   │   ├── token.h/.cpp        # Token types, keywords
│   │   └── lexer.h/.cpp        # Lexer implementation
│   │
│   ├── ast/                    # Phase 2: Abstract Syntax Tree
│   │   ├── ast.h               # 25+ AST node types
│   │   ├── ast.cpp             # Helper implementations
│   │   └── ast_visitor.h       # Visitor interfaces
│   │
│   ├── parser/                 # Phase 2: Parsing
│   │   ├── parser.h            # Parser interface
│   │   └── parser.cpp          # Precedence climbing parser
│   │
│   ├── compiler/               # Phase 3: Bytecode Compilation
│   │   ├── opcode.h            # 40+ VM opcodes
│   │   ├── chunk.h             # Bytecode storage
│   │   ├── compiler.h          # Compiler interface
│   │   └── compiler.cpp        # AST → Bytecode translation
│   │
│   └── vm/                     # Phase 4: Virtual Machine
│   │   ├── object.h            # Runtime objects (Function, Closure, Array, etc.)
│   │   ├── vm.h/.cpp           # Stack-based VM interpreter
│   │
│   └── main.cpp                # CLI entry point (full pipeline)
│
├── build/                      # CMake build output
└── examples/                   # Example Harvis Script files
```

## Implemented Features

### Phase 1: Lexer ✅ (1,579 lines)

**Token Types:**
- **Keywords**: `let`, `const`, `var`, `function`, `class`, `if`, `else`, `while`, `for`, `do`, `break`, `continue`, `return`, `switch`, `case`, `default`, `try`, `catch`, `throw`, `finally`, `import`, `export`, `extends`, `implements`, `new`, `super`, `this`, `true`, `false`, `null`, `undefined`
- **Literals**: Numbers (integers, floats), Strings (single/double-quoted, template literals), Booleans, null, undefined
- **Operators**: 
  - Arithmetic: `+`, `-`, `*`, `/`, `%`, `++`, `--`
  - Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
  - Logical: `&&`, `||`, `!`
  - Assignment: `=`, `+=`, `-=`, `*=`, `/=`, `%=`
  - Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`
  - Ternary: `?:`
  - Optional chaining: `?.`, `??`
- **Delimiters**: `()`, `[]`, `{}`, `,`, `;`, `.`, `..`, `...`, `=>`, `:`
- **Comments**: `// line comments`, `/* block comments */`

**Features:**
- Unicode escape sequences (`\u{...}`, `\uXXXX`)
- Hex escapes (`\xNN`)
- String interpolation (template literals)
- Error reporting with file:line:column

### Phase 2: Parser & AST ✅ (1,700 lines)

**Expression Nodes (20+ types):**
- `LiteralExpr` - Numbers, strings, booleans, null, undefined
- `IdentifierExpr` - Variable references
- `BinaryExpr` - All binary operators
- `UnaryExpr` - `-`, `!`, `~`, `++`, `--`, `typeof`
- `GroupingExpr` - Parenthesized expressions
- `CallExpr` - Function/method calls
- `ArrayLiteralExpr` - `[1, 2, 3]`
- `ObjectLiteralExpr` - `{ key: value }`
- `IndexExpr` - `array[index]`
- `DotExpr` - `object.property`
- `AssignExpr` - `x = 1`, `x += 1`
- `LogicalExpr` - `&&`, `||` (short-circuit)
- `TernaryExpr` - `cond ? a : b`
- `LambdaExpr` - Arrow functions
- `NewExpr` - `new Class()`
- `ThisExpr` - `this`
- `SuperExpr` - `super.method()`
- `TypeCastExpr` - Gradual typing casts

**Statement Nodes (15+ types):**
- `ExpressionStmt` - Expression as statement
- `BlockStmt` - `{ ... }`
- `VarDeclStmt` - `let`, `var`, `const` declarations
- `IfStmt` - `if/else`, `if/else if/else`
- `WhileStmt` - `while` loops
- `ForStmt` - `for` loops (with var/let init)
- `DoWhileStmt` - `do/while` loops
- `ReturnStmt` - `return`
- `BreakStmt` - `break` (labeled)
- `ContinueStmt` - `continue` (labeled)
- `FunctionStmt` - Named functions
- `ClassStmt` - Class definitions with inheritance
- `ImportStmt` - Module imports
- `ExportStmt` - Module exports
- `TryStmt` - Exception handling
- `ThrowStmt` - `throw`
- `SwitchStmt` - `switch/case/default`

**Parser Features:**
- Precedence climbing for expressions
- Error recovery and synchronization
- Gradual typing annotations support
- Full TypeScript-like syntax

### Phase 3: Bytecode Compiler ✅ (800+ lines)

**VM Opcodes (40+ instructions):**

*Constants:*
- `OP_CONST_NULL` - Push null
- `OP_CONST_TRUE` / `OP_CONST_FALSE` - Push booleans
- `OP_CONST_NUMBER` - Push number (with constant pool index)
- `OP_CONST_STRING` - Push string (with constant pool index)
- `OP_CONST_UNDEFINED` - Push undefined

*Variables:*
- `OP_GET_LOCAL` - Load local variable by slot
- `OP_SET_LOCAL` - Store to local variable by slot
- `OP_GET_GLOBAL` - Load global by name
- `OP_SET_GLOBAL` - Store global by name

*Arithmetic:*
- `OP_ADD`, `OP_SUBTRACT`, `OP_MULTIPLY`, `OP_DIVIDE`, `OP_MODULO`
- `OP_NEGATE` - Unary minus

*Comparison:*
- `OP_EQUAL`, `OP_NOT_EQUAL`
- `OP_LESS`, `OP_LESS_EQUAL`, `OP_GREATER`, `OP_GREATER_EQUAL`

*Logical:*
- `OP_AND`, `OP_OR`, `OP_NOT`
- `OP_POP` - Discard top of stack

*Control Flow:*
- `OP_JUMP` - Unconditional jump
- `OP_JUMP_IF_FALSE` - Conditional jump (for `if`, `&&`)
- `OP_JUMP_IF_TRUE` - Conditional jump (for `||`)
- `OP_LOOP` - Backward jump (for `while`, `for`)

*Functions & Objects:*
- `OP_CALL` - Call function
- `OP_RETURN` - Return from function
- `OP_NEW_ARRAY` - Create array
- `OP_NEW_OBJECT` - Create object
- `OP_GET_PROPERTY` / `OP_SET_PROPERTY` - Object property access
- `OP_GET_INDEX` / `OP_SET_INDEX` - Array indexing
- `OP_NEW_CLASS` - Create class
- `OP_METHOD` - Add method to class

*Special:*
- `OP_NOP` - No operation
- `OP_THROW` - Throw exception
- `OP_TRY` / `OP_CATCH` - Exception handling
- `OP_RETURN` - Return

**Compiler Features:**
- Symbol table with scope tracking
- Local variable slot allocation
- Constant pool for strings and numbers
- Jump offset patching
- Error reporting with locations

**Example Compilation:**

```javascript
// Source:
let a = 1 + 2;
let b = 3 * 4;
let c = a + b;

// Bytecode (18 bytes):
   0 | OP_CONST_NUMBER | 1
   2 | OP_ADD          | 
   3 | OP_SET_LOCAL    | 0    ← a = 3
   5 | OP_CONST_NUMBER | 3
   7 | OP_MULTIPLY     | 
   8 | OP_SET_LOCAL    | 1    ← b = 12
  10 | OP_GET_LOCAL    | 0    ← load a
  12 | OP_GET_LOCAL    | 1    ← load b
  14 | OP_ADD          | 
  15 | OP_SET_LOCAL    | 2    ← c = 15
  17 | OP_RETURN       | 

// Constants:
//   0: 1.0
//   1: 2.0
//   2: 3.0
//   3: 4.0
```

## Build & Run

```bash
# Configure and build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run a script
./hs script.hs

# REPL mode
./hs
```

## Examples

### Basic Script

```harvis
// hello.hs
print("Hello, World!");
let x = 42;
print(x);
print(x + 8);
```

**Output:**
```
Hello, World!
42.000000
50.000000
```

### Arithmetic & Variables

```harvis
let a = 10;
let b = 20;
let sum = a + b;
let product = a * b;
print("Sum: " + sum);
print("Product: " + product);
```

### String Operations

```harvis
let name = "Harvis";
let greeting = "Welcome, " + name + "!";
print(greeting);
let length = 5;
print(length);
```

## Bytecode Format

Harvis Script compiles to a compact stack-based bytecode format:

```
OP_GET_GLOBAL 0      ; Push global "print"
OP_CONST_STRING 1    ; Push string "Hello"
OP_CALL 1            ; Call with 1 argument
OP_POP             ; Discard result
OP_RETURN          ; Return
```

### Constant Pool
Constants are stored in a pool and referenced by index:
- **Strings**: Internationalized, deduplicated
- **Numbers**: 64-bit IEEE 754 doubles
- **Names**: Global variable names for OP_GET/SET_GLOBAL

### Stack Operations
- **Push**: Constants, locals, globals, computation results
- **Pop**: Discard values, retrieve arguments
- **Peek**: Access stack without removing

## VM Architecture

### Runtime Objects
```cpp
RuntimeObject {
    ObjectType type;      // Function, Closure, Native, Array, Map, String
    Type declaredType;    // Gradual type info
}

Function : RuntimeObject {
    string name;
    Chunk chunk;          // Bytecode
    int arity;            // Parameter count
}

NativeFunction : RuntimeObject {
    string name;
    std::function<Value(Value*, int)> function;
}
```

### Memory Management
- **Value**: Tagged union with RAII semantics
- **Strings**: Heap-allocated, copy-on-write
- **Objects**: VM-managed with double-free protection
- **Stack**: Pre-allocated (65535 slots) for performance


## Next Phases

### Phase 4: Stack VM ✅ (650+ lines)

**Runtime Objects:**
- `Function` - Compiled functions with bytecode
- `Closure` - Functions with captured upvalues
- `NativeFunction` - C++ implemented functions
- `ArrayObj` - Dynamic arrays
- `MapObj` - Key-value objects
- `StringObj` - String objects

**VM Features:**
- Stack-based bytecode execution
- Global variable table
- Native functions (`print`, `println`)
- Memory-safe with double-free protection
- Stack overflow/underflow detection

**Example Execution:**
```harvis
// Source:
print("Hello, World!");
let x = 42;
print(x + 8);

// Output:
Hello, World!
50.000000
```

### Phase 5: Standard Library 🚧
- ✅ Basic I/O (print, println)
- ⏳ Math functions (Math.*)
- ⏳ String operations (String.*)
- ⏳ Array methods (push, pop, map, filter, reduce)

### Phase 6: Closures & Upvalues 🚧
- ⏳ Closure objects
- ⏳ Upvalue capture
- ⏳ Nested function support

### Phase 7: Garbage Collection 🚧
- ⏳ Reference counting or mark-and-sweep
- ⏳ Object lifecycle management

### Phase 8: Type System 🚧
- ⏳ Gradual type checking
- ⏳ Type inference
- ⏳ Type errors at compile time

## Design Decisions

1. **C++17**: Modern C++ with wide support, no C++20/23 dependencies
2. **Stack-based VM**: Simpler implementation, denser bytecode, easier to extend
3. **Gradual Typing**: Like TypeScript - optional type annotations, dynamic at runtime
4. **Visitor Pattern**: Clean separation between AST nodes and operations
5. **Precedence Climbing**: Efficient expression parsing without recursion
6. **Symbol Tables**: Proper scoping with local variable slots
7. **Memory Safety**: No double-frees, no leaks, RAII throughout

## Contributing

### Development Setup
```bash
git clone https://github.com/yourusername/compiler.git
cd compiler
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Running Tests
```bash
# Basic functionality test
echo 'print("Hello, World!");' | ./hs

# With AddressSanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make -j$(nproc)
./hs script.hs
```

### Code Style
- C++17 compliant
- No raw new/delete (use RAII)
- Use std::optional, std::variant, std::string_view
- Zero warnings policy (compile with `-Wall -Wextra`)

## Changelog

### v0.1.0 (Current)
- ✅ Lexer: Complete tokenization with error reporting
- ✅ Parser: Full TypeScript-like syntax support
- ✅ Compiler: Bytecode generation with symbol tables
- ✅ VM: Stack-based execution with native functions
- ✅ Memory safety: No leaks, no double-frees
- ✅ Zero warnings build

### Roadmap
- [ ] Standard library (Math, String, Array)
- [ ] Closures and upvalues
- [ ] Garbage collection
- [ ] Type checking
- [ ] Module system
- [ ] AOT compilation

## License

MIT License - See LICENSE file for details
