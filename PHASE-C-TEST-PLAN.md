# Phase C: Testing & Debugging - Test Plan

## 1. Architecture Overview

The Harvis Script compiler consists of 4 phases:
1. **Lexer** — Tokenizes source code into tokens
2. **Parser** — Parses tokens into AST (visitor pattern)
3. **Compiler** — Emits bytecode to chunks
4. **VM** — Interprets bytecode with stack machine

## 2. Feature Coverage

### 2.1 Basic Operations
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparisons: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`, `!`
- Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`

### 2.2 Types
- Numbers, Strings, Booleans
- `null`, `undefined`
- Arrays, Objects/Maps

### 2.3 Control Flow
- If/else, while, for, do-while
- Switch statement
- Range-based iteration (`range`)

### 2.4 Functions & Classes
- Function declarations
- Class with methods
- Constructor
- `this`, `super`

### 2.5 Advanced Features (Phase A fixes)
- **Try/Catch/Throw** — Exception handling
- **Select statement** — Channel multiplexing
- **Goroutines** — Concurrent execution with `go`
- **Import/Export** — Module system

## 3. Known Edge Cases & Potential Issues

### 3.1 Lexer Edge Cases
- Unicode escape sequences (`\u{XXXX}`)
- Hex escapes (`\xXX`)
- Template strings with embedded expressions
- Block comment termination
- Invalid character handling

### 3.2 Parser Edge Cases
- Empty array: `[]` vs `[1,]`
- Trailing commas in arrays/objects
- Method shorthand in objects
- Type annotations with generics

### 3.3 Compiler Edge Cases
- **Variable resolution priority** — local vs global ambiguity
- **Jump offset calculation** — 16-bit signed offset for loops
- **Closure capture** — capturing stack variables
- **Try/catch stack unwinding** — complex control flow

### 3.4 VM Edge Cases
- **Stack underflow** — missing operands
- **Type errors** — wrong operand types
- **Channel deadlock** — receive from empty channel
- **Goroutine race conditions** — concurrent stack access
- **Null reference** — property access on null

### 3.5 Memory Management
- Reference counting for objects
- String duplication vs sharing
- Closure upvalue handling

## 4. Test Files to Create

### 4.1 Basic Tests (`test_basic/`)
| File | Tests |
|------|-------|
| `01_arithmetic.hs` | +, -, *, /, %, precedence |
| `02_comparison.hs` | ==, !=, <, >, <=, >= |
| `03_logical.hs` | &&, \|\|, !, truthiness |
| `04_strings.hs` | concat, length, indexing |
| `05_arrays.hs` | literal, index, len() |
| `06_objects.hs` | literal, property access |

### 4.2 Control Flow Tests (`test_control/`)
| File | Tests |
|------|-------|
| `01_if_else.hs` | if/else branches |
| `02_while.hs` | while loop |
| `03_for.hs` | for loop |
| `04_do_while.hs` | do-while at least once |
| `05_switch.hs` | switch cases, default |
| `06_break_continue.hs` | break/continue in loops |
| `07_range.hs` | range iteration |

### 4.3 Functions & Classes Tests (`test_functions/`)
| File | Tests |
|------|-------|
| `01_function_decl.hs` | function definition/call |
| `02_recursion.hs` | recursive functions |
| `03_closure.hs` | closure capturing |
| `04_class.hs` | class declaration |
| `05_method.hs` | instance methods |
| `06_constructor.hs` | constructor invocation |
| `07_this.hs` | this reference |

### 4.4 Exception Handling Tests (`test_exceptions/`)
| File | Tests |
|------|-------|
| `01_throw.hs` | throw statements |
| `02_try_catch.hs` | try/catch basic |
| `03_try_finally.hs` | finally always runs |
| `04_nested_try.hs` | nested try/catch |
| `05_recover.hs` | recover after panic |

### 4.5 Concurrency Tests (`test_concurrency/`)
| File | Tests |
|------|-------|
| `01_channel.hs` | channel send/receive |
| `02_buffered_channel.hs` | buffered channels |
| `03_goroutine.hs` | go statement |
| `04_select.hs` | select statement |
| `05_select_default.hs` | default case in select |
| `06_mutex.hs` | mutex lock/unlock |
| `07_waitgroup.hs` | waitgroup sync |

### 4.6 Module Tests (`test_modules/`)
| File | Tests |
|------|-------|
| `01_import.hs` | import statements |
| `02_named_import.hs` | named imports |
| `03_namespace_import.hs` | * as namespace |
| `04_export.hs` | export statements |

### 4.7 Edge Case Tests (`test_edge/`)
| File | Tests |
|------|-------|
| `01_empty_array.hs` | [] array |
| `02_empty_object.hs` | {} object |
| `03_null_property.hs` | null.x access |
| `04_undefined_prop.hs` | undefined.x |
| `05_optional_chain.hs` | ?. operator |
| `06_nullish_coalesce.hs` | ?? operator |
| `07_type_assertion.hs` | as type assertion |
| `08_type_check.hs` | type checking |

### 4.8 Stress Tests (`test_stress/`)
| File | Tests |
|------|-------|
| `01_deep_recursion.hs` | deep call stack |
| `02_large_loop.hs` | many iterations |
| `03_many_goroutines.hs` | concurrent load |

## 5. Test Execution Strategy

1. **Compile** each test file
2. **Run** the compiled bytecode
3. **Capture** output and errors
4. **Compare** against expected results

## 6. Defect Categories

| Category | Risk | Detection |
|----------|------|----------|
| Stack underflow | High | Run test with insufficient operands |
| Type confusion | Medium | Run with wrong types |
| Channel deadlock | Medium | Run with no sender |
| Race condition | Medium | Run with multiple goroutines |
| Memory leak | Low | Long-running tests |
| Jump overflow | Medium | Deeply nested loops |

## 7. Expected Test Output Format

Each test file should produce:
- Console output (expected)
- Exit code (0 = success)
- No runtime errors