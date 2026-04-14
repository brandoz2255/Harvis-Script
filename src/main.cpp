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
            std::cout << "constant[" << (int)chunk.code[i + 1] << "]";
            i++;
        } else if (op == Opcode::OP_GET_LOCAL || op == Opcode::OP_SET_LOCAL || 
                   op == Opcode::OP_CALL || op == Opcode::OP_GET_GLOBAL || op == Opcode::OP_SET_GLOBAL) {
            i++;
            std::cout << (int)chunk.code[i];
        } else if (op == Opcode::OP_JUMP || op == Opcode::OP_JUMP_IF_FALSE || 
                   op == Opcode::OP_JUMP_IF_TRUE || op == Opcode::OP_LOOP) {
            i++;
            std::cout << "to " << (i + chunk.code[i]);
        } else {
            std::cout << " ";
        }
        std::cout << "\n";
        i++;
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
