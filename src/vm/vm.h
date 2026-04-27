#ifndef HARVIS_VM_VM_H
#define HARVIS_VM_VM_H

#include "../compiler/chunk.h"
#include "../compiler/opcode.h"
#include "object.h"
#include <vector>
#include <memory>
#include <stack>
#include <unordered_map>
#include <thread>

namespace hs {

class VM {
public:
    VM();
    ~VM();
    
    // Run bytecode
    void run(Function* function);
    
    // Get last result
    Value lastResult() const { return lastValue; }
    
    // Error handling
    bool hasError() const { return error; }
    const std::string& getError() const { return errorMessage; }
    
private:
    std::vector<Value> stack;
    std::vector<std::shared_ptr<Upvalue>> upvalues;
    std::unordered_map<std::string, Value> globals;
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
        std::vector<Value> locals;  // Separate local variable storage
    };
    
    std::stack<CallFrame> callStack;
    
    // Defer support
    struct DeferCall {
        Value callable;
        std::vector<Value> args;
    };
    std::vector<DeferCall> deferredCallStack;

    // Module system
    std::unordered_map<std::string, Value> modules;
    MapObj* currentModule;

    // Try/Catch support
    struct TryFrame {
        int tryBodyStart;    // IP where try body begins
        int catchIP;         // IP of OP_CATCH
        int stackDepth;      // stack depth at try start
        int callDepth;       // callStack depth at try start
    };
    std::vector<TryFrame> tryStack;

    // Panic support
    Value panicValue;
    bool panicActive;
    
    // Recover support
    Value recoverValue;
    
       // Module loading
    Value loadModule(const std::string& moduleName);
    std::unordered_map<std::string, Value>& getGlobals() { return globals; }
    
    // Execution
    bool interpret();
    Value executeCall(int argCount);
    bool call(Closure* closure, int argCount);
    bool invoke(Instance* instance, const std::string& name, int argCount);
    
    // Upvalues
    std::shared_ptr<Upvalue> captureUpvalue(int local);
    void closeUpvalues(int localIndex);
    
    // Goroutine synchronization
    std::recursive_mutex vmMutex;

    // Native functions
    void registerNatives();

    // Helper
    void reportError(const std::string& message);
    void executeDeferred();
};

} // namespace hs

#endif // HARVIS_VM_VM_H
