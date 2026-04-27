# Phase 14: Built-in Functions - GOAL

## Goal Statement
Implement Harvis Script's built-in functions and range loop syntax to provide essential runtime utilities and concise iteration over collections.

## Success Metrics
- `len(collection)` returns length of arrays, strings, and maps
- `make(type, params)` allocates initialized arrays, maps, and channels
- `append(array, element)` returns new array with element added
- `copy(dest, src)` copies elements between arrays
- `delete(map, key)` removes key from map
- `range` loop syntax works with arrays, strings, maps, and channels
- Type assertion (`expr as Type`) works for runtime type checking
- Type switch statement works for multi-way type dispatch
- All existing tests continue to pass

## Out of Scope
- Standard library functions beyond built-ins
- Full reflection API (type introspection only for assertions)
- Custom allocation functions
