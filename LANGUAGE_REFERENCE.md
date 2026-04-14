# Harvis Script Language Reference

## Quick Start

```hs
// Variables
let x = 42;
let name = "Harvis";
const VERSION = "1.0.0";

// Functions
function greet(name) {
    return "Hello, " + name + "!";
}

// Arrow functions
let add = (a, b) => a + b;

// Control flow
if (x > 10) {
    print("x is greater than 10");
} else {
    print("x is small");
}

// Loops
for (let i = 0; i < 10; i++) {
    print(i);
}

// Classes
class Person {
    constructor(name, age) {
        this.name = name;
        this.age = age;
    }
    
    greet() {
        return "Hi, I'm " + this.name;
    }
}

let alice = new Person("Alice", 30);
print(alice.greet());
```

## Variables

### let (block-scoped, mutable)
```hs
let x = 10;
x = 20;  // OK
```

### const (block-scoped, immutable)
```hs
const PI = 3.14159;
// PI = 3.0;  // Error: cannot reassign const
```

### var (function-scoped, mutable) - legacy
```hs
var legacy = "use let instead";
```

## Data Types

### Primitives
```hs
let number = 42;           // Number (double-precision float)
let float = 3.14159;       // Float
let scientific = 1.5e10;   // Scientific notation

let text = "Hello";        // String (UTF-8)
let alsoText = 'World';    // Single-quoted string

let enabled = true;        // Boolean
let disabled = false;

let nothing = null;        // Null
let undefinedVal = undefined;  // Undefined
```

### Escape Sequences
```hs
let path = "C:\Users\test";    // Raw
let newline = "Line1\nLine2";  // \n = newline
let tab = "Col1\tCol2";        // \t = tab
let quote = "Say \"Hello\"";   // \" = double quote
let unicode = "\u{1F600}";     // 😀
let hex = "\x41";              // A
```

### Template Literals
```hs
let name = "World";
let greeting = `Hello, ${name}!`;  // "Hello, World!"
let multiline = `Line 1
Line 2
Line 3`;
```

### Arrays
```hs
let numbers = [1, 2, 3, 4, 5];
let mixed = [1, "two", true, null];
let nested = [[1, 2], [3, 4]];

// Access
let first = numbers[0];      // 1
let last = numbers[numbers.length - 1];  // 5

// Modify
numbers[0] = 10;             // [10, 2, 3, 4, 5]
numbers.push(6);             // [10, 2, 3, 4, 5, 6]
```

### Objects
```hs
let person = {
    name: "Alice",
    age: 30,
    active: true,
    greet: function() {
        return "Hello, " + this.name;
    }
};

// Access
print(person.name);          // "Alice"
print(person["age"]);        // 30
print(person.greet());       // "Hello, Alice"

// Modify
person.age = 31;
person.email = "alice@example.com";
```

## Operators

### Arithmetic
```hs
let a = 10, b = 3;
10 + 3   // 13  (addition)
10 - 3   // 7   (subtraction)
10 * 3   // 30  (multiplication)
10 / 3   // 3.33... (division)
10 % 3   // 1   (modulo)
++a      // increment
--b      // decrement
```

### Comparison
```hs
10 == 10    // true  (equal)
10 != 5     // true  (not equal)
10 > 5      // true  (greater than)
10 >= 10    // true  (greater or equal)
5 < 10      // true  (less than)
5 <= 5      // true  (less or equal)
```

### Logical
```hs
true && false   // false (AND - short-circuit)
true || false   // true  (OR - short-circuit)
!true           // false (NOT)

// Short-circuit examples
let value = null;
let result = value || "default";  // "default"

let enabled = true;
let config = enabled && getOptions();  // Only calls getOptions() if enabled
```

### Assignment
```hs
let x = 10;
x += 5;   // x = x + 5  → 15
x -= 3;   // x = x - 3  → 12
x *= 2;   // x = x * 2  → 24
x /= 4;   // x = x / 4  → 6
x %= 4;   // x = x % 4  → 2
```

### Ternary
```hs
let age = 20;
let status = age >= 18 ? "adult" : "minor";  // "adult"
```

## Control Flow

### if / else
```hs
let score = 85;

if (score >= 90) {
    print("A");
} else if (score >= 80) {
    print("B");
} else if (score >= 70) {
    print("C");
} else {
    print("F");
}
```

### switch
```hs
let day = 3;

switch (day) {
    case 1:
        print("Monday");
        break;
    case 2:
        print("Tuesday");
        break;
    case 3:
        print("Wednesday");
        break;
    default:
        print("Other day");
}
```

### while
```hs
let i = 0;
while (i < 5) {
    print(i);
    i++;
}
```

### do / while
```hs
let j = 0;
do {
    print(j);
    j++;
} while (j < 5);
```

### for
```hs
// Traditional
for (let i = 0; i < 10; i++) {
    print(i);
}

// For-each (when supported)
for (let item of items) {
    print(item);
}
```

### break / continue
```hs
for (let i = 0; i < 10; i++) {
    if (i % 2 == 0) continue;  // Skip even numbers
    if (i > 7) break;          // Stop at 8
    print(i);                  // 1, 3, 5, 7
}

// Labeled breaks
outer: for (let i = 0; i < 3; i++) {
    for (let j = 0; j < 3; j++) {
        if (i == 1 && j == 1) break outer;
    }
}
```

## Functions

### Function Declaration
```hs
function add(a, b) {
    return a + b;
}

function greet(name = "Guest") {
    return "Hello, " + name + "!";
}
```

### Arrow Functions
```hs
let multiply = (a, b) => a * b;

let square = x => x * x;  // Single param, no parens

let identity = x => x;    // Implicit return

let verbose = (a, b) => {
    let sum = a + b;
    return sum;
};
```

### Recursion
```hs
function factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
```

### Closures (when supported)
```hs
function makeCounter() {
    let count = 0;
    return () => {
        count++;
        return count;
    };
}

let counter = makeCounter();
print(counter());  // 1
print(counter());  // 2
```

## Classes

### Basic Class
```hs
class Person {
    constructor(name, age) {
        this.name = name;
        this.age = age;
    }
    
    greet() {
        return "Hi, I'm " + this.name;
    }
    
    isAdult() {
        return this.age >= 18;
    }
}

let alice = new Person("Alice", 25);
print(alice.greet());      // "Hi, I'm Alice"
print(alice.isAdult());    // true
```

### Inheritance
```hs
class Student extends Person {
    constructor(name, age, grade) {
        super(name, age);
        this.grade = grade;
    }
    
    study() {
        return this.name + " is studying grade " + this.grade;
    }
}

let bob = new Student("Bob", 16, 10);
print(bob.greet());   // "Hi, I'm Bob" (inherited)
print(bob.study());   // "Bob is studying grade 10"
```

## Exception Handling

```hs
try {
    let result = riskyOperation();
    print("Success: " + result);
} catch (error) {
    print("Error: " + error);
} finally {
    print("Cleanup");
}
```

## Modules

### Export
```hs
// math.hs
export const PI = 3.14159;

export function add(a, b) {
    return a + b;
}

export default function multiply(a, b) {
    return a * b;
}
```

### Import
```hs
// main.hs
import multiply, { PI, add } from "math";

print(PI);           // 3.14159
print(add(2, 3));    // 5
print(multiply(2, 3)); // 6

// Namespace import
import * as Math from "math";
print(Math.PI);
print(Math.add(2, 3));
```

## Built-in Functions (Planned)

```hs
// Console
print(value);        // Print to stdout
println(value);      // Print with newline

// Math
abs(x);              // Absolute value
max(a, b);           // Maximum
min(a, b);           // Minimum
random();            // Random 0-1
sqrt(x);             // Square root
pow(x, y);           // Power
round(x);            // Round
floor(x);            // Floor
ceil(x);             // Ceiling

// String
length(str);         // String length
substring(str, start, end);
toUpperCase(str);
toLowerCase(str);
trim(str);
split(str, delimiter);

// Array
push(arr, value);    // Push to array
pop(arr);            // Pop from array
length(arr);         // Array length

// Type checking
typeof(x);           // "number", "string", "boolean", etc.
isNumber(x);         // true/false
isString(x);         // true/false
isArray(x);          // true/false
```

## Comments

```hs
// Single-line comment

/* Multi-line
   comment */

/// Documentation comment (when supported)
```

## Reserved Keywords

```
let const var
function class constructor
if else switch case default
while for do break continue
return yield
true false null undefined
import export from as
extends implements
try catch throw finally
new this super
public private protected
static abstract final
type interface enum
```
