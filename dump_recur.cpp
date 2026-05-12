
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include "src/compiler/compiler.h"
#include "src/parser/parser.h"
#include "src/lexer/lexer.h"

using namespace hs;

void dumpChunk(const Chunk* chunk, const std::string& name) {
    std::cout << "=== " << name << " (" << chunk->code.size() << " bytes) ===\n";
    int ip = 0;
    while (ip < (int)chunk->code.size()) {
        int start = ip;
        Opcode op = (Opcode)chunk->code[ip++];
        std::string opname;
        switch(op) {
            case Opcode::OP_CLOSURE: {
                int funcIdx = chunk->code[ip++];
                int uvCount = chunk->code[ip++];
                std::cout << "  " << start << ": OP_CLOSURE(func=" << funcIdx << ", uv=" << uvCount << ")\n";
                break;
            }
            case Opcode::OP_GET_LOCAL:
                std::cout << "  " << start << ": OP_GET_LOCAL(" << (int)chunk->code[ip++] << ")\n";
                break;
            case Opcode::OP_SET_LOCAL:
                std::cout << "  " << start << ": OP_SET_LOCAL(" << (int)chunk->code[ip++] << ")\n";
                break;
            case Opcode::OP_GET_GLOBAL:
                std::cout << "  " << start << ": OP_GET_GLOBAL(" << (int)chunk->code[ip++] << ")\n";
                break;
            case Opcode::OP_SET_GLOBAL:
                std::cout << "  " << start << ": OP_SET_GLOBAL(" << (int)chunk->code[ip++] << ")\n";
                break;
            case Opcode::OP_CONST_NUMBER:
                std::cout << "  " << start << ": OP_CONST_NUMBER(" << chunk->constants[chunk->code[ip++]].toNumber() << ")\n";
                break;
            case Opcode::OP_CONST_STRING:
                std::cout << "  " << start << ": OP_CONST_STRING(" << chunk->constants[chunk->code[ip++]].toString() << ")\n";
                break;
            case Opcode::OP_CONST_NULL:
                std::cout << "  " << start << ": OP_CONST_NULL\n";
                break;
            case Opcode::OP_CONST_TRUE:
                std::cout << "  " << start << ": OP_CONST_TRUE\n";
                break;
            case Opcode::OP_CONST_FALSE:
                std::cout << "  " << start << ": OP_CONST_FALSE\n";
                break;
            case Opcode::OP_CONST_UNDEFINED:
                std::cout << "  " << start << ": OP_CONST_UNDEFINED\n";
                break;
            case Opcode::OP_ADD:
                std::cout << "  " << start << ": OP_ADD\n";
                break;
            case Opcode::OP_SUBTRACT:
                std::cout << "  " << start << ": OP_SUBTRACT\n";
                break;
            case Opcode::OP_MULTIPLY:
                std::cout << "  " << start << ": OP_MULTIPLY\n";
                break;
            case Opcode::OP_DIVIDE:
                std::cout << "  " << start << ": OP_DIVIDE\n";
                break;
            case Opcode::OP_EQUAL:
                std::cout << "  " << start << ": OP_EQUAL\n";
                break;
            case Opcode::OP_NOT_EQUAL:
                std::cout << "  " << start << ": OP_NOT_EQUAL\n";
                break;
            case Opcode::OP_LESS:
                std::cout << "  " << start << ": OP_LESS\n";
                break;
            case Opcode::OP_LESS_EQUAL:
                std::cout << "  " << start << ": OP_LESS_EQUAL\n";
                break;
            case Opcode::OP_GREATER:
                std::cout << "  " << start << ": OP_GREATER\n";
                break;
            case Opcode::OP_GREATER_EQUAL:
                std::cout << "  " << start << ": OP_GREATER_EQUAL\n";
                break;
            case Opcode::OP_NOT:
                std::cout << "  " << start << ": OP_NOT\n";
                break;
            case Opcode::OP_AND:
                std::cout << "  " << start << ": OP_AND\n";
                break;
            case Opcode::OP_OR:
                std::cout << "  " << start << ": OP_OR\n";
                break;
            case Opcode::OP_CALL:
                std::cout << "  " << start << ": OP_CALL(" << (int)chunk->code[ip++] << ")\n";
                break;
            case Opcode::OP_RETURN:
                std::cout << "  " << start << ": OP_RETURN\n";
                break;
            case Opcode::OP_POP:
                std::cout << "  " << start << ": OP_POP\n";
                break;
            case Opcode::OP_JUMP: {
                int low = chunk->code[ip++];
                int high = chunk->code[ip++];
                int offset = low | (high << 8);
                if (offset & 0x8000) offset -= 0x10000;
                std::cout << "  " << start << ": OP_JUMP(" << offset << ")\n";
                break;
            }
            case Opcode::OP_JUMP_IF_FALSE: {
                int low = chunk->code[ip++];
                int high = chunk->code[ip++];
                int offset = low | (high << 8);
                if (offset & 0x8000) offset -= 0x10000;
                std::cout << "  " << start << ": OP_JUMP_IF_FALSE(" << offset << ")\n";
                break;
            }
            case Opcode::OP_JUMP_IF_TRUE: {
                int low = chunk->code[ip++];
                int high = chunk->code[ip++];
                int offset = low | (high << 8);
                if (offset & 0x8000) offset -= 0x10000;
                std::cout << "  " << start << ": OP_JUMP_IF_TRUE(" << offset << ")\n";
                break;
            }
            case Opcode::OP_LOOP: {
                int low = chunk->code[ip++];
                int high = chunk->code[ip++];
                int offset = low | (high << 8);
                if (offset & 0x8000) offset -= 0x10000;
                std::cout << "  " << start << ": OP_LOOP(" << offset << ")\n";
                break;
            }
            case Opcode::OP_DEFERRED_RETURN:
                std::cout << "  " << start << ": OP_DEFERRED_RETURN\n";
                break;
            case Opcode::OP_DUP:
                std::cout << "  " << start << ": OP_DUP\n";
                break;
            case Opcode::OP_CLOSE_UPVIEW:
                std::cout << "  " << start << ": OP_CLOSE_UPVALUE\n";
                break;
            case Opcode::OP_NEW_ARRAY:
                std::cout << "  " << start << ": OP_NEW_ARRAY(" << (int)chunk->code[ip++] << ")\n";
                break;
            case Opcode::OP_NEW_OBJECT:
                std::cout << "  " << start << ": OP_NEW_OBJECT(" << (int)chunk->code[ip++] << ")\n";
                break;
            case Opcode::OP_NEW_STRUCT:
                std::cout << "  " << start << ": OP_NEW_STRUCT(" << (int)chunk->code[ip++] << ", " << (int)chunk->code[ip++] << ")\n";
                break;
            default:
                std::cout << "  " << start << ": OP_" << (int)op << " (unknown)\n";
                break;
        }
    }
}

int main() {
    std::ifstream f("/tmp/test_recur.hs");
    if (!f) {
        std::cerr << "Cannot open /tmp/test_recur.hs\n";
        return 1;
    }
    std::string src((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    
    Parser parser(tokens);
    auto program = parser.parse();
    
    Compiler compiler;
    for (auto& stmt : program.statements) {
        stmt->accept(compiler);
    }
    
    // Get main chunk via reflection
    std::cout << "=== main chunk ===\n";
    auto* mainChunk = &compiler.chunk;
    
    for (int i = 0; i < (int)mainChunk->code.size(); i++) {
        if (mainChunk->code[i] == (int)Opcode::OP_CLOSURE) {
            int funcIdx = mainChunk->code[i+1];
            int uvCount = mainChunk->code[i+2];
            std::cout << "\nFound OP_CLOSURE at ip=" << i << " funcIdx=" << funcIdx << " uv=" << uvCount << "\n";
            
            // Find the function constant
            for (auto& c : mainChunk->constants) {
                if (c.isObject() && c.asObject()) {
                    // Check if it's a Function
                    auto* obj = c.asObject();
                    if (obj->type == hs::ObjectType::Function) {
                        auto* fn = (Function*)obj;
                        std::cout << "Function: " << fn->name << "\n";
                        std::cout << "Chunk size: " << fn->chunk.code.size() << "\n";
                        dumpChunk(&fn->chunk, fn->name);
                    }
                }
            }
        }
    }
    
    return 0;
}
