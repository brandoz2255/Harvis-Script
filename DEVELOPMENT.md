# Harvis Script Developer Guide

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Harvis Script Compiler                   │
├─────────────────────────────────────────────────────────────┤
│  Source Code                                                 │
│       ↓                                                      │
│  ┌─────────┐    Tokens    ┌─────────┐    AST    ┌─────────┐ │
│  │ Lexer   │─────────────>│ Parser  │─────────>│Compiler │ │
│  └─────────┘              └─────────┘          └─────────┘ │
│       ↑                    ↑                ↓              │
│       │                    │            Bytecode          │
│  Error Reporting        Syntax Tree  ┌─────────┐         │
│                                     │   VM    │         │
│                                     └─────────┘         │
│                                          ↓              │
│                                     Execution            │
└─────────────────────────────────────────────────────────────┘
```

## Building the Project

### Prerequisites
- CMake 3.16+
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)

### Build Steps

```bash
# Clone repository
git clone https://github.com/yourusername/compiler.git
cd compiler

# Configure
mkdir build && cd build
cmake ..

# Build (Debug mode)
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Build (Release mode)
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Run tests (when implemented)
make check

# Install
sudo make install
```

## Running the Compiler

```bash
# Compile and show tokens
./build/hs example.hs

# Output includes:
# 1. Tokens (lexed output)
# 2. AST (parsed structure)
# 3. Bytecode (compiled instructions)
```

## Code Organization

### Core Module (`src/core/`)
```
types.h/.cpp  → Runtime values (Value, Type, Scope, Environment)
error.h       → Error handling (CompileError, Result<T>)
scanner.h     → Character buffer for source scanning
```

**Key Concepts:**
- `Value`: Tagged union for runtime values (number, string, boolean, object)
- `Type`: Gradual typing information
- `Scope`: Variable scope management
- `Result<T>`: Error handling without exceptions

### Lexer Module (`src/lexer/`)
```
token.h/.cpp  → Token definitions, keyword table
lexer.h/.cpp  → Lexer implementation
```

**Key Concepts:**
- `TokenType`: Enum of all token types
- `Token`: Token with type, lexeme, location
- `Lexer::tokenize()`: Main entry point
- `CharBuffer`: Efficient character scanning

### AST Module (`src/ast/`)
```
ast.h         → 25+ AST node definitions
ast.cpp       → Helper implementations
ast_visitor.h → Visitor interfaces
```

**Key Concepts:**
- Visitor pattern for AST traversal
- Expression nodes (Expr) vs Statement nodes (Stmt)
- Source location tracking for errors

### Parser Module (`src/parser/`)
```
parser.h/.cpp → Recursive descent parser
```

**Key Concepts:**
- Precedence climbing for expressions
- Error recovery with synchronization
- Token lookahead and matching

### Compiler Module (`src/compiler/`)
```
opcode.h      → VM instruction set
chunk.h       → Bytecode storage
compiler.h/.cpp → AST to bytecode translation
```

**Key Concepts:**
- Symbol table with scope tracking
- Constant pool for literals
- Jump offset patching
- Local variable slot allocation

## Adding New Features

### Adding a New Token Type

1. Add to `TokenType` enum in `src/lexer/token.h`
2. Add keyword to `KeywordTable` in `src/lexer/token.cpp`
3. Add handling in `Lexer::scanToken()` in `src/lexer/lexer.cpp`
4. Add to `tokenTypeToString()` for debugging

### Adding a New AST Node

1. Define class in `src/ast/ast.h`:
```cpp
class MyNewExpr : public Expr {
public:
    // Fields
    Expr::Ptr operand;
    
    MyNewExpr(SourceLocation loc, Expr::Ptr op)
        : Expr(loc), operand(std::move(op)) {}
    
    void accept(Visitor& v) override { v.visitMyNewExpr(this); }
    std::string toString() const override { return "MyNewExpr(...)"; }
};
```

2. Add visitor method in `Visitor` and `Compiler` classes

3. Implement parser method in `src/parser/parser.cpp`

4. Implement compiler visitor in `src/compiler/compiler.cpp`

### Adding a New Opcode

1. Add to `Opcode` enum in `src/compiler/opcode.h`
2. Add to `opcodeName()` switch in same file
3. Implement in VM (Phase 4)

### Adding Built-in Functions

Will be implemented in Phase 5 (stdlib).

## Debugging Tips

### Enable Verbose Output

```cpp
// In main.cpp, after compilation:
printBytecode(*compiler.getChunk());
```

### Common Issues

1. **Memory leaks**: Check `scopes` vector in compiler
2. **Infinite loops**: Usually in `parseCall()` or expression parsing
3. **Slot numbering**: Ensure `localCount` starts at 0
4. **Jump offsets**: Debug with bytecode dump

### Testing Strategy

1. **Lexer tests**: Tokenize known inputs, verify tokens
2. **Parser tests**: Parse known inputs, verify AST structure
3. **Compiler tests**: Compile known inputs, verify bytecode
4. **VM tests**: Execute bytecode, verify output

## Performance Considerations

### Current Optimizations
- CharBuffer for efficient source scanning
- Constant pool to avoid duplicate strings/numbers
- Local variable slots (faster than global lookup)

### Future Optimizations
- Constant folding (compile-time evaluation)
- Dead code elimination
- Jump table optimization for switch statements
- Inlining small functions

## File Line Counts

```
src/lexer/       → 1,579 lines
src/parser/      →   900 lines
src/ast/         →   800 lines
src/compiler/    →   800 lines
src/core/        →   700 lines
────────────────────────────────
Total            → 4,779 lines
```

## Contributing

1. Fork the repository
2. Create feature branch
3. Make changes
4. Add tests
5. Run `make check`
6. Submit pull request

## License

MIT License - See LICENSE file

## References

- [Crafting Interpreters](https://craftinginterpreters.com/) - Robert Nystrom
- [Language Implementation Patterns](https://www.manning.com/books/language-implementation-patterns) - Terence Parr
- Lua source code - Excellent VM design
- V8 source code - Advanced optimization techniques
