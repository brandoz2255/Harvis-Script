#ifndef HARVIS_COMPILER_OPCODE_H
#define HARVIS_COMPILER_OPCODE_H

#include <cstdint>
#include <string>

namespace hs {

// Bytecode opcodes for stack-based VM
enum class Opcode : uint8_t {
    // Constants
    OP_CONST_NULL,          // Push null
    OP_CONST_TRUE,          // Push true
    OP_CONST_FALSE,         // Push false
    OP_CONST_NUMBER,        // Push number (followed by double)
    OP_CONST_STRING,        // Push string (followed by string index)
    OP_CONST_UNDEFINED,     // Push undefined
    
    // Variables
    OP_GET_LOCAL,           // Pop index, push local var
    OP_SET_LOCAL,           // Pop value, pop index, set local var
    OP_GET_GLOBAL,          // Pop name, push global var
    OP_SET_GLOBAL,          // Pop value, pop name, set global
    OP_GET_UPVALUE,         // Pop index, push upvalue
    OP_SET_UPVALUE,         // Pop value, pop index, set upvalue
    
    // Arithmetic
    OP_ADD,                 // Pop b, pop a, push a + b
    OP_SUBTRACT,            // Pop b, pop a, push a - b
    OP_MULTIPLY,            // Pop b, pop a, push a * b
    OP_DIVIDE,              // Pop b, pop a, push a / b
    OP_MODULO,              // Pop b, pop a, push a % b
    OP_NEGATE,              // Pop a, push -a
    
    // Comparison
    OP_EQUAL,               // Pop b, pop a, push a == b
    OP_NOT_EQUAL,           // Pop b, pop a, push a != b
    OP_LESS,                // Pop b, pop a, push a < b
    OP_LESS_EQUAL,          // Pop b, pop a, push a <= b
    OP_GREATER,             // Pop b, pop a, push a > b
    OP_GREATER_EQUAL,       // Pop b, pop a, push a >= b
    
    // Logical
    OP_AND,                 // Pop b, pop a, push a && b (short-circuit)
    OP_OR,                  // Pop b, pop a, push a || b (short-circuit)
    OP_NOT,                 // Pop a, push !a
    
    // Bitwise
    OP_BITWISE_AND,         // Pop b, pop a, push a & b
    OP_BITWISE_OR,          // Pop b, pop a, push a | b
    OP_BITWISE_XOR,         // Pop b, pop a, push a ^ b
    OP_BITWISE_NOT,         // Pop a, push ~a
    OP_SHIFT_LEFT,          // Pop b, pop a, push a << b
    OP_SHIFT_RIGHT,         // Pop b, pop a, push a >> b
    
    OP_POP,                 // Pop top of stack
    OP_DUP,                 // Duplicate top of stack
    
    // Control flow
    OP_JUMP,                // Pop offset, jump to offset
    OP_JUMP_IF_FALSE,       // Pop offset, pop condition, jump if false
    OP_JUMP_IF_TRUE,        // Pop offset, pop condition, jump if true
    OP_LOOP,                // Pop offset, jump back to offset
    OP_NULLISH_COALESCE,    // Jump to right operand if left is not null/undefined
    
    // Functions
    OP_CALL,                // Pop args..., pop fn, call fn
    OP_RETURN,              // Pop value, return to caller
    OP_CLOSURE,             // Push closure (captures upvalues)
    OP_CLOSE_UPVALUE,       // Convert top stack slot to upvalue
    OP_CONST_FUNCTION,      // Push function from constant pool
    
    // Objects
    OP_GET_PROPERTY,        // Pop name, pop obj, push obj[name]
    OP_SET_PROPERTY,        // Pop value, pop name, pop obj, set obj[name]
    OP_GET_INDEX,           // Pop index, pop arr, push arr[index]
    OP_SET_INDEX,           // Pop value, pop index, pop arr, set arr[index]
    OP_NEW_ARRAY,           // Pop elements..., pop count, create array
    OP_NEW_OBJECT,          // Pop properties..., pop count, create object
    OP_ARRAY_PUSH,          // Pop value, pop arr, arr.push(value)
    OP_ARRAY_POP,           // Pop arr, push arr.pop()
    
    // Classes
    OP_NEW_CLASS,           // Push class object
    OP_NEW_INSTANCE,        // Pop class, create instance
    OP_INHERIT,             // Pop base, pop derived, set up base
    OP_METHOD,              // Pop method, pop name, bind method to class
    OP_NEW_CLASS_GENERIC,   // argCount, typeArgCount: pop args, typeArgs, className, ctorName, create instance
    OP_CALL_GENERIC,        // argCount, typeArgCount: pop args, typeArgs, fn, call
    
    // I/O
    OP_PRINT,               // Pop value, print to stdout
    OP_READ,                // Read from stdin, push value
    
    // Structs
    OP_NEW_STRUCT,           // Pop n values, pop structName, pop fieldCount, create struct, push struct
    OP_GET_FIELD,            // Pop nameIdx, pop struct, push struct[nameIdx]
    OP_SET_FIELD,            // Pop value, pop nameIdx, pop struct, struct[nameIdx] = value
    OP_GET_FIELD_OPTIONAL,   // Pop nameIdx, pop obj, push obj[nameIdx] or undefined if obj is null/undefined
    OP_GET_INDEX_OPTIONAL,   // Pop index, pop arr, push arr[index] or undefined if arr is null/undefined
    
    // Built-in functions
    OP_LEN,                 // Pop collection, push length
    OP_MAKE,                // Pop type + params, push allocated object
    OP_APPEND,              // Pop element, pop array, push new array
    OP_COPY,                // Pop dest, pop src, push count
    OP_DELETE,              // Pop key, pop map, push success bool
    OP_IMPORT,              // Pop module name, push imported values
    OP_EXPORT,              // Mark previous value as exported
    
    // Range loop
    OP_RANGE_ARRAY,       // Pop array, push iterator
    OP_RANGE_NEXT,        // Pop iterator, push (hasNext, value)
    OP_RANGE_STRING,      // Pop string, push iterator
    OP_RANGE_MAP,         // Pop map, push iterator
    OP_RANGE_MAP_NEXT,    // Pop iterator, push (hasNext, key, value)
    
    // Array helpers
    OP_ARRAY_LENGTH,    // Pop array, push length
    OP_ARRAY_AT,        // Pop index, pop array, push array[index]
    
    // Type checking
    OP_TYPE_CHECK,    // Pop typeIdx, pop value, push bool (type matches)
    
    // Defer
    OP_DEFER,                // Pop callable, push to defer stack
    OP_DEFERRED_RETURN,      // Execute deferred calls in LIFO order
    
    // Panic/Recover
    OP_PANIC,                // Pop value, start panic unwinding
    OP_RECOVER,              // Return panic value or undefined
    
    // Interfaces
    OP_IS_INSTANCE,          // Pop nameIdx, pop value, push bool (is instance of interface)
    OP_INVOKE,               // Pop methodNameIdx, pop args..., pop receiver, call method
    
    // Goroutines
    OP_GO,                  // Pop function + args, spawn new goroutine
    OP_MAKE_CHANNEL,        // Pop capacity, push channel object
    OP_SEND,                // Pop value, pop channel, send value (blocking)
    OP_RECEIVE,             // Pop channel, push value (blocking)
    OP_SELECT_CASE,         // Start of select case (pushes channel+value onto stack)
    OP_SELECT_DEFAULT,      // Start of select default case
    OP_SELECT_CASE_END,     // End of select case body
    OP_SELECT,              // Pop branches, execute first ready branch
    
    // Sync primitives
    OP_MUTEX_NEW,           // Push new mutex object
    OP_MUTEX_LOCK,          // Pop mutex, lock mutex
    OP_MUTEX_UNLOCK,        // Pop mutex, unlock mutex
    OP_WAITGROUP_NEW,       // Push new waitgroup object
    OP_WAITGROUP_ADD,       // Pop delta, pop waitgroup, add delta to counter
    OP_WAITGROUP_WAIT,      // Pop waitgroup, block until counter reaches 0
    
    // Error handling
    OP_THROW,               // Pop value, throw exception
    OP_TRY,                 // Mark try block start
    OP_CATCH,               // Mark catch block
    
    // Debug
    OP_NOP,                 // No operation
    OP_DEBUG,               // Debug breakpoint
    
    // End of function
    OP_END                  // Mark end of bytecode
};

// Get opcode name for debugging
inline const char* opcodeName(Opcode op) {
    switch (op) {
#define OP(name) case Opcode::name: return #name;
        OP(OP_CONST_NULL)
        OP(OP_CONST_TRUE)
        OP(OP_CONST_FALSE)
        OP(OP_CONST_NUMBER)
        OP(OP_CONST_STRING)
        OP(OP_CONST_UNDEFINED)
        OP(OP_GET_LOCAL)
        OP(OP_SET_LOCAL)
        OP(OP_GET_GLOBAL)
        OP(OP_SET_GLOBAL)
        OP(OP_GET_UPVALUE)
        OP(OP_SET_UPVALUE)
        OP(OP_ADD)
        OP(OP_SUBTRACT)
        OP(OP_MULTIPLY)
        OP(OP_DIVIDE)
        OP(OP_MODULO)
        OP(OP_NEGATE)
        OP(OP_EQUAL)
        OP(OP_NOT_EQUAL)
        OP(OP_LESS)
        OP(OP_LESS_EQUAL)
        OP(OP_GREATER)
        OP(OP_GREATER_EQUAL)
        OP(OP_AND)
        OP(OP_OR)
        OP(OP_NOT)
        OP(OP_BITWISE_AND)
        OP(OP_BITWISE_OR)
        OP(OP_BITWISE_XOR)
        OP(OP_BITWISE_NOT)
        OP(OP_SHIFT_LEFT)
        OP(OP_SHIFT_RIGHT)
        OP(OP_POP)
        OP(OP_DUP)
        OP(OP_JUMP)
        OP(OP_JUMP_IF_FALSE)
        OP(OP_JUMP_IF_TRUE)
        OP(OP_LOOP)
        OP(OP_CALL)
        OP(OP_RETURN)
        OP(OP_CLOSURE)
        OP(OP_CLOSE_UPVALUE)
        OP(OP_CONST_FUNCTION)
        OP(OP_GET_PROPERTY)
        OP(OP_SET_PROPERTY)
        OP(OP_GET_INDEX)
        OP(OP_SET_INDEX)
        OP(OP_NEW_ARRAY)
        OP(OP_NEW_OBJECT)
        OP(OP_ARRAY_PUSH)
        OP(OP_ARRAY_POP)
        OP(OP_NEW_CLASS)
        OP(OP_NEW_INSTANCE)
        OP(OP_INHERIT)
        OP(OP_METHOD)
        OP(OP_NEW_CLASS_GENERIC)
        OP(OP_CALL_GENERIC)
        OP(OP_NEW_STRUCT)
        OP(OP_GET_FIELD)
        OP(OP_SET_FIELD)
        OP(OP_GET_FIELD_OPTIONAL)
        OP(OP_GET_INDEX_OPTIONAL)
        OP(OP_NULLISH_COALESCE)
        OP(OP_LEN)
        OP(OP_MAKE)
        OP(OP_APPEND)
        OP(OP_COPY)
        OP(OP_DELETE)
        OP(OP_IMPORT)
        OP(OP_EXPORT)
        OP(OP_RANGE_ARRAY)
        OP(OP_RANGE_NEXT)
        OP(OP_RANGE_STRING)
        OP(OP_RANGE_MAP)
        OP(OP_RANGE_MAP_NEXT)
        OP(OP_ARRAY_LENGTH)
        OP(OP_ARRAY_AT)
        OP(OP_TYPE_CHECK)
        OP(OP_DEFER)
        OP(OP_DEFERRED_RETURN)
        OP(OP_PANIC)
        OP(OP_RECOVER)
         OP(OP_IS_INSTANCE)
        OP(OP_INVOKE)
        OP(OP_GO)
        OP(OP_MAKE_CHANNEL)
        OP(OP_SEND)
        OP(OP_RECEIVE)
        OP(OP_SELECT_CASE)
        OP(OP_SELECT_DEFAULT)
        OP(OP_SELECT_CASE_END)
        OP(OP_SELECT)
        OP(OP_MUTEX_NEW)
        OP(OP_MUTEX_LOCK)
        OP(OP_MUTEX_UNLOCK)
        OP(OP_WAITGROUP_NEW)
        OP(OP_WAITGROUP_ADD)
        OP(OP_WAITGROUP_WAIT)
        OP(OP_PRINT)
        OP(OP_READ)
        OP(OP_THROW)
        OP(OP_TRY)
        OP(OP_CATCH)
        OP(OP_NOP)
        OP(OP_DEBUG)
        OP(OP_END)
#undef OP
        default: return "UNKNOWN";
    }
}

} // namespace hs

#endif // HARVIS_COMPILER_OPCODE_H
