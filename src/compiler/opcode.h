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
    OP_POP,                 // Pop top of stack
    
    // Control flow
    OP_JUMP,                // Pop offset, jump to offset
    OP_JUMP_IF_FALSE,       // Pop offset, pop condition, jump if false
    OP_JUMP_IF_TRUE,        // Pop offset, pop condition, jump if true
    OP_LOOP,                // Pop offset, jump back to offset
    
    // Functions
    OP_CALL,                // Pop args..., pop fn, call fn
    OP_RETURN,              // Pop value, return to caller
    OP_CLOSURE,             // Push closure (captures upvalues)
    OP_CLOSE_UPVALUE,       // Convert top stack slot to upvalue
    
    // Objects
    OP_GET_PROPERTY,        // Pop name, pop obj, push obj[name]
    OP_SET_PROPERTY,        // Pop value, pop name, pop obj, set obj[name]
    OP_GET_INDEX,           // Pop index, pop arr, push arr[index]
    OP_SET_INDEX,           // Pop value, pop index, pop arr, set arr[index]
    OP_NEW_ARRAY,           // Pop elements..., pop count, create array
    OP_NEW_OBJECT,          // Pop properties..., pop count, create object
    
    // Classes
    OP_NEW_CLASS,           // Push class object
    OP_INHERIT,             // Pop base, pop derived, set up base
    OP_METHOD,              // Push method onto class
    
    // I/O
    OP_PRINT,               // Pop value, print to stdout
    OP_READ,                // Read from stdin, push value
    
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
        OP(OP_POP)
        OP(OP_JUMP)
        OP(OP_JUMP_IF_FALSE)
        OP(OP_JUMP_IF_TRUE)
        OP(OP_LOOP)
        OP(OP_CALL)
        OP(OP_RETURN)
        OP(OP_CLOSURE)
        OP(OP_CLOSE_UPVALUE)
        OP(OP_GET_PROPERTY)
        OP(OP_SET_PROPERTY)
        OP(OP_GET_INDEX)
        OP(OP_SET_INDEX)
        OP(OP_NEW_ARRAY)
        OP(OP_NEW_OBJECT)
        OP(OP_NEW_CLASS)
        OP(OP_INHERIT)
        OP(OP_METHOD)
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
