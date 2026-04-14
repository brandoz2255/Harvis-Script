# Harvis Script - Current Syntax (v0.1)

## What Actually Works Right Now

This document describes the **actual implemented syntax** as of the current build, not planned features.

## ✅ Fully Implemented

### Variables

```hs
// let (block-scoped, mutable)
let x = 42;
let name = "Harvis";
let pi = 3.14159;
let enabled = true;

// const (block-scoped, immutable)
const VERSION = "1.0.0";

// var (function-scoped) - works but use let instead
var legacy = "old style";
```

### Literals

```hs
// Numbers
let int = 42;
let float = 3.14159;
let scientific = 1.5e10;
let negative = -42;

// Strings
let doubleQuoted = "Hello";
let singleQuoted = 'World';
let withNewline = "Line1\nLine2";
let withTab = "Col1\tCol2";
let withQuote = "Say \"Hello\"";
let hex = "\x41";           // A
let unicode = "\u{1F600}";  // 😀

// Booleans
let yes = true;
let no = false;

// Null/Undefined
let nothing = null;
let undefinedVal = undefined;
```

### Arrays

```hs
let numbers = [1, 2, 3, 4, 5];
let mixed = [1, "two", true, null];
let nested = [[1, 2], [3, 4]];

// Access
let first = numbers[0];
let last = numbers[2];
```

### Objects

```hs
let person = {
    name: "Alice",
    age: 30,
    active: true
};

// Property access
let n = person.name;
let a = person.age;
```

### Arithmetic Operators

```hs
let a = 10, b = 3;

let sum = a + b;        // 13
let diff = a - b;       // 7
let prod = a * b;       // 30
let quot = a / b;       // 3.33...
let rem = a % b;        // 1
let neg = -a;           // -10
```

### Comparison Operators

```hs
let eq = (10 == 10);       // true
let neq = (10 != 5);       // true
let lt = (5 < 10);         // true
let lte = (10 <= 10);      // true
let gt = (10 > 5);         // true
let gte = (10 >= 10);      // true
```

### Logical Operators

```hs
let andResult = true && false;  // false
let orResult = true || false;   // true
let notResult = !true;          // false
```

### Ternary Operator

```hs
let age = 20;
let status = age >= 18 ? "adult" : "minor";
```

### Assignment

```hs
let x = 10;
x = 20;           // Reassignment
x += 5;           // x = x + 5
x -= 3;           // x = x - 3
x *= 2;           // x = x * 2
x /= 4;           // x = x / 4
x %= 4;           // x = x % 4
```

### Control Flow

```hs
// if / else if / else
let score = 85;
if (score >= 90) {
    // A
} else if (score >= 80) {
    // B
} else {
    // F
}

// while
let i = 0;
while (i < 5) {
    i++;
}

// do / while
let j = 0;
do {
    j++;
} while (j < 5);

// for
for (let k = 0; k < 10; k++) {
    // Loop body
}

// for with var declaration
for (let m = 0; m < 5; m = m + 1) {
    // Loop body
}

// switch / case / default
let day = 3;
switch (day) {
    case 1:
        // Monday
        break;
    case 2:
        // Tuesday
        break;
    default:
        // Other
}
```

### Functions

```hs
// Function declaration
function add(a, b) {
    return a + b;
}

// Function with default parameter
function greet(name) {
    if (name == null) {
        name = "Guest";
    }
    return "Hello, " + name;
}

// Arrow function (syntax accepted, limited support)
let multiply = (a, b) => a * b;
```

### Classes

```hs
class Person {
    constructor(name, age) {
        this.name = name;
        this.age = age;
    }
    
    greet() {
        return "Hi, I'm " + this.name;
    }
}

// Class instantiation
let alice = new Person("Alice", 25);
let greeting = alice.greet();
```

### Exception Handling

```hs
try {
    // Risky operation
    let result = dangerousFunction();
} catch (error) {
    // Handle error
    print("Error: " + error);
} finally {
    // Cleanup
}

// Throw
throw "Something went wrong";
```

### Comments

```hs
// Single-line comment

/* Multi-line
   comment */
```

## ⚠️ Partially Implemented

These work syntactically but have limited runtime support:

### `this` Keyword
```hs
class MyClass {
    constructor() {
        this.value = 42;  // Syntax accepted
    }
}
```

### `super` Keyword
```hs
class Child extends Parent {
    constructor() {
        super();  // Syntax accepted
    }
}
```

### Break / Continue
```hs
for (let i = 0; i < 10; i++) {
    if (i == 5) break;      // Syntax accepted
    if (i % 2 == 0) continue;  // Syntax accepted
}
```

### Import / Export
```hs
export function hello() {
    return "world";
}

export const VERSION = "1.0.0";

import { hello } from "module";
```

## ❌ Not Yet Implemented

These will be added in future phases:

- **VM execution** - Bytecode compiles but doesn't run yet
- **Standard library** - `print()`, `read()`, etc.
- **Closures** - Full closure support with upvalues
- **Garbage collection** - Manual memory management for now
- **Type checking** - Gradual typing exists but not enforced
- **Generators** - `yield` keyword not implemented
- **Async/await** - Not yet supported
- **Template literals** - `${...}` interpolation not working
- **Array methods** - `.push()`, `.pop()`, etc.
- **String methods** - `.length`, `.substring()`, etc.

## Bytecode Output

The compiler generates stack-based bytecode:

```hs
// Source:
let a = 1 + 2;
let b = 3 * 4;

// Bytecode:
   0 | OP_CONST_NUMBER | 1       // Push 1
   2 | OP_CONST_NUMBER | 2       // Push 2
   4 | OP_ADD          |         // Add (result: 3)
   5 | OP_SET_LOCAL    | 0       // Store in slot 0 (a)
   7 | OP_CONST_NUMBER | 3       // Push 3
   9 | OP_CONST_NUMBER | 4       // Push 4
  11 | OP_MULTIPLY     |         // Multiply (result: 12)
  12 | OP_SET_LOCAL    | 1       // Store in slot 1 (b)
  14 | OP_RETURN       |         // Return
```

## Error Handling

The compiler reports errors with locations:

```
script.hs:5:10: error: Undefined variable: x
script.hs:10:5: error: Variable already declared in this scope
```

## Testing Your Code

```bash
# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Test
cd ..
./build/hs your_script.hs
```

Output shows:
1. **Tokens** - Lexed output
2. **AST** - Parsed structure
3. **Bytecode** - Compiled instructions

## Known Limitations

1. **No runtime yet** - Compiler works, VM coming in Phase 4
2. **No standard library** - Built-in functions not implemented
3. **Limited closure support** - Nested functions partially working
4. **No type enforcement** - Type annotations ignored at runtime
5. **Memory management** - No garbage collection yet

## Next Phases

- **Phase 4**: Stack VM interpreter
- **Phase 5**: Standard library
- **Phase 6**: Full closure support
- **Phase 7**: Garbage collection
- **Phase 8**: Type system enforcement
