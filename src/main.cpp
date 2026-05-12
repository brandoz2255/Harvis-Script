#include "lexer/lexer.h"
#include "lexer/token.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "compiler/compiler.h"
#include "compiler/opcode.h"
#include "compiler/chunk.h"
#include "vm/vm.h"
#include "vm/object.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace hs;

void printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << token << " @ " << token.location.toString() << "\n";
    }
}

void printAST(const Program& program) {
    if (!program.package.empty()) {
        std::cout << "  package " << program.package << "\n";
    }
    for (const auto& stmt : program.statements) {
        std::cout << "  " << stmt->toString() << "\n";
    }
}

void printBytecode(const Chunk& chunk) {
    std::cout << "\n=== Bytecode ===\n";
    for (size_t i = 0; i < chunk.code.size(); ) {
        auto op = static_cast<Opcode>(chunk.code[i]);
        std::cout << std::setw(4) << i << " | " << std::setw(25) << opcodeName(op) << " | ";
        
        if (op == Opcode::OP_CONST_NUMBER || op == Opcode::OP_CONST_STRING) {
            if (i + 1 < chunk.code.size()) {
                std::cout << "constant[" << (int)chunk.code[i + 1] << "]";
            }
            i += 2;
            std::cout << "\n";
            continue;
        } else if (op == Opcode::OP_CLOSURE) {
            int upvalueCount = 0;
            if (i + 2 < chunk.code.size()) {
                upvalueCount = chunk.code[i + 2];
            }
            std::cout << "func[" << (int)chunk.code[i + 1] << "] upvalues[" << upvalueCount << "]";
            i += 3 + upvalueCount;
            std::cout << "\n";
            continue;
        } else if (op == Opcode::OP_CONST_FUNCTION || op == Opcode::OP_GET_UPVALUE || 
                   op == Opcode::OP_SET_UPVALUE || op == Opcode::OP_CLOSE_UPVALUE) {
            if (i + 1 < chunk.code.size()) {
                std::cout << "index[" << (int)chunk.code[i + 1] << "]";
            }
            i += 2;
            std::cout << "\n";
            continue;
        } else if (op == Opcode::OP_GET_LOCAL || op == Opcode::OP_SET_LOCAL || 
                   op == Opcode::OP_CALL || op == Opcode::OP_GET_GLOBAL || op == Opcode::OP_SET_GLOBAL) {
            if (i + 1 < chunk.code.size()) {
                std::cout << (int)chunk.code[i + 1];
            }
            i += 2;
            std::cout << "\n";
            continue;
} else if (op == Opcode::OP_JUMP || op == Opcode::OP_JUMP_IF_FALSE || 
                   op == Opcode::OP_JUMP_IF_TRUE || op == Opcode::OP_LOOP) {
            int offset = 0;
            if (i + 2 < chunk.code.size()) {
                offset = chunk.code[i + 1] | (chunk.code[i + 2] << 8);
                if (offset & 0x8000) offset -= 0x10000;
            }
            std::cout << "to " << (i + 2 + offset);
            i += 3;
            std::cout << "\n";
            continue;
        } else if (op == Opcode::OP_NEW_ARRAY || op == Opcode::OP_NEW_OBJECT) {
            if (i + 1 < chunk.code.size()) {
                std::cout << "count[" << (int)chunk.code[i + 1] << "]";
            }
            i += 2;
            std::cout << "\n";
            continue;
        } else if (op == Opcode::OP_NEW_STRUCT) {
            if (i + 2 < chunk.code.size()) {
                int nameIdx = chunk.code[i + 1];
                int fieldCount = chunk.code[i + 2];
                std::cout << "struct[" << nameIdx << "] fields[" << fieldCount << "]";
                i += 3 + fieldCount;
                std::cout << "\n";
                continue;
            } else {
                i += 3;
                std::cout << "\n";
                continue;
            }
        } else if (op == Opcode::OP_GET_FIELD || op == Opcode::OP_SET_FIELD || 
                   op == Opcode::OP_IS_INSTANCE || op == Opcode::OP_INVOKE || op == Opcode::OP_CLOSE_UPVALUE ||
                   op == Opcode::OP_GET_FIELD_OPTIONAL) {
            if (i + 1 < chunk.code.size()) {
                std::cout << "index[" << (int)chunk.code[i + 1] << "]";
            }
            i += 2;
            std::cout << "\n";
            continue;
} else if (op == Opcode::OP_GET_INDEX_OPTIONAL || op == Opcode::OP_NULLISH_COALESCE) {
             std::cout << " ";
             i++;
             std::cout << "\n";
             continue;
        } else if (op == Opcode::OP_TYPE_CHECK) {
             if (i + 1 < chunk.code.size()) {
                 std::cout << "typeIdx[" << (int)chunk.code[i + 1] << "]";
             }
             i += 2;
             std::cout << "\n";
             continue;
       } else if (op == Opcode::OP_EXPORT) {
             if (i + 1 < chunk.code.size()) {
                 std::cout << "nameIdx[" << (int)chunk.code[i + 1] << "]";
             }
             i += 2;
             std::cout << "\n";
             continue;
        } else if (op == Opcode::OP_DEFER || op == Opcode::OP_DEFERRED_RETURN ||
                     op == Opcode::OP_PANIC || op == Opcode::OP_RECOVER ||
                     op == Opcode::OP_SELECT_CASE || op == Opcode::OP_SELECT_DEFAULT ||
                     op == Opcode::OP_SELECT_CASE_END || op == Opcode::OP_SEND ||
                     op == Opcode::OP_RECEIVE || op == Opcode::OP_MAKE_CHANNEL ||
                     op == Opcode::OP_SELECT || op == Opcode::OP_GO ||
                     op == Opcode::OP_MUTEX_NEW || op == Opcode::OP_MUTEX_LOCK ||
                     op == Opcode::OP_MUTEX_UNLOCK || op == Opcode::OP_WAITGROUP_NEW ||
                     op == Opcode::OP_WAITGROUP_ADD || op == Opcode::OP_WAITGROUP_WAIT) {
            std::cout << " ";
            i++;
            std::cout << "\n";
            continue;
       } else {
            std::cout << " ";
            i++;
            std::cout << "\n";
        }
    }
    std::cout << "Constants:\n";
    for (size_t i = 0; i < chunk.constants.size(); i++) {
        std::cout << "  " << std::setw(3) << i << ": " << chunk.constants[i].toString() << "\n";
    }
}

int interpret(const std::string& source, const std::string& filename = "<script>") {
    Lexer lexer(source, filename);
    auto tokens = lexer.tokenize();
    
    if (lexer.hasErrors()) {
        std::cerr << "\n=== Lexer Errors ===\n";
        for (const auto& error : lexer.getErrors()) {
            std::cerr << error.what() << "\n";
        }
        return 1;
    }
    
    std::cout << "\n=== Tokens ===\n";
    printTokens(tokens);
    
    // Parse
    Parser parser(tokens);
    auto program = parser.parse();
    
    if (parser.hasErrors()) {
        std::cerr << "\n=== Parser Errors ===\n";
        for (const auto& error : parser.getErrors()) {
            std::cerr << error.what() << "\n";
        }
        return 1;
    }
    
    std::cout << "\n=== AST ===\n";
    printAST(program);
    
    // Compile
    Compiler compiler;
    compiler.compile(program);
    
    if (compiler.hasErrors()) {
        std::cerr << "\n=== Compiler Errors ===\n";
    }
    
    std::cout << "\n=== Bytecode (size: " << compiler.getChunk()->code.size() << " bytes) ===\n";
    printBytecode(*compiler.getChunk());
    
    if (compiler.hasErrors()) {
        return 1;
    }
    
    // Execute with VM
    std::cout << "\n=== Execution ===\n";
    VM vm;
    
    // Create Function object for VM
    Chunk* chunk = compiler.getChunk();
    auto* func = new Function(filename, *chunk, 0, true, false);
    
    vm.run(func);
    
    if (vm.hasError()) {
        std::cerr << "\nRuntime Error: " << vm.getError() << "\n";
        delete func;
        return 1;
    }
    
    std::cout << "Final result: " << vm.lastResult().toString() << "\n";
    delete func;
    return 0;
}

int runFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << "\n";
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    return interpret(source, filename);
}

void repl() {
    std::cout << "Harvis Script REPL\n";
    std::cout << "Type 'exit' to quit\n\n";
    
    while (true) {
        std::cout << "hs> ";
        std::string line;
        std::getline(std::cin, line);
        
        if (line == "exit" || line == "quit") {
            break;
        }
        
        if (!line.empty()) {
            interpret(line, "<repl>");
            std::cout << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: hs [script.hs]\n";
        std::cerr << "  Without arguments: start REPL\n";
        std::cerr << "  With filename: execute script file\n";
        return 1;
    } else if (argc == 2) {
        return runFile(argv[1]);
    } else {
        repl();
        return 0;
    }
}
