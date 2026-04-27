# Phase 12: Generics - DESIGN

## Overview
Add generic type parameters to struct declarations, function declarations, and generic collections (Array<T>, Map<K,V>).

## Design Decisions

### 1. Monomorphization at Compile Time
Each concrete instantiation of a generic type gets a unique compiled form. For structs that compile to MapObj, we generate unique struct names per type argument combination.

### 2. Syntax
```
// Generic struct
struct Box<T> {
    value: T
}

// Generic struct with multiple type params
struct Pair<T, U> {
    first: T
    second: U
}

// Generic function
func identity<T>(x: T): T {
    return x
}

// Generic instantiation
let box = Box<number> { value: 42 }
let pair = Pair<number, string> { first: 1, second: "hello" }
```

### 3. Type System
- `Type::params` field already exists but is unused — we'll populate it
- Generic types are resolved at compile time, no runtime overhead
- Type parameters are replaced with concrete types during compilation

### 4. Runtime
- Structs remain MapObj at runtime
- Generic struct instantiations create unique MapObj variants with mangled names
- No new object types needed — monomorphization handles everything at compile time

### 5. Constraints (Phase 12 scope)
- Type parameters are unconstrained (any type allowed)
- Type constraints on type parameters deferred to later phase
