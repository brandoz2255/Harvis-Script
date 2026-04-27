# Phase 12: Generics - GOAL

## Goal Statement
Implement compile-time generics for Harvis Script, enabling type-parameterized structs and functions with monomorphization, making the language safer and more expressive without runtime performance overhead.

## Success Metrics
- Generic structs compile and instantiate correctly with concrete types
- Generic functions accept and return any type, with explicit type arguments
- Array<T> syntax works for typed arrays
- No runtime performance penalty (monomorphization is compile-time)
- All existing tests continue to pass

## Out of Scope
- Type constraints (e.g., `T where T: Comparable`)
- Type inference (explicit type arguments required)
- Generic methods on classes
- Standard library generic collections (ArrayObj, MapObj as generic objects)
