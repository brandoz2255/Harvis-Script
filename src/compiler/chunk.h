#ifndef HARVIS_COMPILER_CHUNK_H
#define HARVIS_COMPILER_CHUNK_H

#include "opcode.h"
#include "../core/types.h"
#include <vector>
#include <memory>
#include <stdexcept>

namespace hs {

// A chunk of bytecode
struct Chunk {
    std::vector<uint8_t> code;      // Bytecode instructions
    std::vector<Value> constants;   // Constant pool
    std::vector<std::string> lines; // Line number info for debugging

    void write(uint8_t byte, int line) {
        code.push_back(byte);
        lines.push_back(std::to_string(line));
    }

    void write(Opcode op, int line) {
        (void)line;
        code.push_back(static_cast<uint8_t>(op));
        lines.push_back(std::string(opcodeName(op)));
    }

    void writeDouble(double value, int line) {
        (void)line;
        code.push_back(static_cast<uint8_t>(Opcode::OP_CONST_NUMBER));
        constants.push_back(Value(value));
        lines.push_back(std::to_string(static_cast<int>(value)));
    }

    void writeString(const std::string& value, int line) {
        (void)line;
        code.push_back(static_cast<uint8_t>(Opcode::OP_CONST_STRING));
        constants.push_back(Value(value));
        lines.push_back(value);
    }
    
    // Add a string constant and return its index
    int addStringConstant(const std::string& value) {
        int index = static_cast<int>(constants.size());
        constants.push_back(Value(value));
        return index;
    }

    // Patch a previous jump offset
    void patchJump(int offset, int target) {
        int jumpTarget = target - (code.size() - offset);
        if (jumpTarget < 0 || jumpTarget > 255) {
            throw std::runtime_error("Jump offset out of range");
        }
        code[code.size() - offset] = static_cast<uint8_t>(jumpTarget);
    }
};

// Compiled function (for compiler)
struct CompiledFunction {
    std::string name;
    Chunk chunk;
    int arity;
    bool isModule;
    bool isClassMethod;
};

// Compiler context
struct CompilerContext {
    Chunk* currentChunk;
    std::vector<CompiledFunction*> functions;
    std::vector<std::string> strings;
    
    CompilerContext() : currentChunk(nullptr) {}
    
    void beginFunction(std::string fnName) {
        functions.push_back(new CompiledFunction());
        functions.back()->name = fnName;
        functions.back()->arity = 0;
        functions.back()->isModule = false;
        functions.back()->isClassMethod = false;
        currentChunk = &functions.back()->chunk;
    }
    
    void endFunction() {
        if (!functions.empty()) {
            delete functions.back();
            functions.pop_back();
            if (!functions.empty()) {
                currentChunk = &functions.back()->chunk;
            } else {
                currentChunk = nullptr;
            }
        }
    }
};

} // namespace hs

#endif // HARVIS_COMPILER_CHUNK_H
