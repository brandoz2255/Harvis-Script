#ifndef HARVIS_VM_VM_H
#define HARVIS_VM_VM_H

#include "../compiler/chunk.h"
#include "../compiler/opcode.h"
#include "object.h"
#include <vector>
#include <memory>
#include <stack>
#include <unordered_map>

namespace hs {

class VM {
public:
    VM();
    ~VM();
    
    // Run bytecode
    void run(Function* function);
    void runChunk(Chunk& chunk);
    
    // Get last result
    Value lastResult() const { return lastValue; }
    
    // Error handling
    bool hasError() const { return error; }
    const std::string& getError() const { return errorMessage; }
    
private:
    std::vector<Value> stack;
    std::vector<std::shared_ptr<Upvalue>> upvalues;
    std::unordered_map<std::string, Value> globals;
    std::vector<Value> frames[64];  // Stack frames
    int frameCount;
    
    bool error;
    std::string errorMessage;
    Value lastValue;
    
    // Stack operations
    Value pop();
    void push(Value value);
    Value peek(int offset);
    void defineGlobal(const std::string& name, Value value);
    Value getGlobal(const std::string& name);
    void setGlobal(const std::string& name, Value value);
    
    // Call frame
    struct CallFrame {
        Function* function;
        Chunk* chunk;
        int ip;  // Instruction pointer
        int stackStart;
    };
    
    std::stack<CallFrame> callStack;
    
    // Execution
    bool interpret();
    Value executeCall(int argCount);
    bool call(Closure* closure, int argCount);
    bool invoke(Instance* instance, const std::string& name, int argCount);
    
    // Upvalues
    std::shared_ptr<Upvalue> captureUpvalue(int local);
    void closeUpvalues(int localIndex);
    
    // Native functions
    void registerNatives();
    
    // Helper
    void reportError(const std::string& message);
};

} // namespace hs

#endif // HARVIS_VM_VM_H
