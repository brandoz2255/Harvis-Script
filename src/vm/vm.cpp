#include "vm.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <unordered_set>

namespace hs {

VM::VM() : frameCount(0), error(false) {
    stack.reserve(65535);
    registerNatives();
}

VM::~VM() {
    std::unordered_set<RuntimeObject*> deleted;
    
    // Clean up globals
    for (auto& pair : globals) {
        if (pair.second.isObject() && pair.second.asObject()) {
            auto* obj = pair.second.asObject();
            if (deleted.find(obj) == deleted.end()) {
                deleted.insert(obj);
                delete obj;
            }
        }
    }
    
    // Clean up stack
    for (auto& value : stack) {
        if (value.isObject() && value.asObject()) {
            auto* obj = value.asObject();
            if (deleted.find(obj) == deleted.end()) {
                deleted.insert(obj);
                delete obj;
            }
        }
    }
    
    // Clean up upvalues
    for (auto& upvalue : upvalues) {
        if (upvalue && upvalue->closed) {
            if (upvalue->closed->isObject() && upvalue->closed->asObject()) {
                auto* obj = upvalue->closed->asObject();
                if (deleted.find(obj) == deleted.end()) {
                    deleted.insert(obj);
                    delete obj;
                }
            }
        }
    }
}

void VM::push(Value value) {
    if (stack.size() >= stack.capacity()) {
        reportError("Stack overflow.");
        return;
    }
    stack.push_back(std::move(value));
}

Value VM::pop() {
    if (stack.empty()) {
        reportError("Stack underflow.");
        return Value();
    }
    Value value = std::move(stack.back());
    stack.pop_back();
    return value;
}

Value VM::peek(int offset) {
    return stack[stack.size() - 1 - offset];
}

void VM::defineGlobal(const std::string& name, Value value) {
    globals[name] = value;
}

Value VM::getGlobal(const std::string& name) {
    auto it = globals.find(name);
    if (it == globals.end()) {
        reportError("Undefined variable: " + name);
        return Value();
    }
    return it->second;
}

void VM::setGlobal(const std::string& name, Value value) {
    auto it = globals.find(name);
    if (it == globals.end()) {
        reportError("Undefined variable: " + name);
        return;
    }
    it->second = value;
}

std::shared_ptr<Upvalue> VM::captureUpvalue(int local) {
    // Check if already an upvalue
    for (auto& upvalue : upvalues) {
        if (upvalue->slot == &stack[local]) {
            return upvalue;
        }
    }
    
    // Create new upvalue
    auto upvalue = std::make_shared<Upvalue>(&stack[local]);
    upvalues.push_back(upvalue);
    return upvalue;
}

void VM::closeUpvalues(int localIndex) {
    while (!upvalues.empty() && upvalues.back()->slot >= &stack[localIndex]) {
        upvalues.back()->close();
        upvalues.pop_back();
    }
}

void VM::registerNatives() {
    // print function
    auto* printFn = new NativeFunction("print", 
        [](Value* stack, int argCount) -> Value {
            for (int i = 0; i < argCount; i++) {
                if (i > 0) printf(" ");
                printf("%s", stack[i].toString().c_str());
            }
            printf("\n");
            return Value();
        },
        -1);
    defineGlobal("print", Value(printFn));
    
    // println function
    auto* printlnFn = new NativeFunction("println",
        [](Value* stack, int argCount) -> Value {
            for (int i = 0; i < argCount; i++) {
                if (i > 0) printf(" ");
                printf("%s", stack[argCount - 1 - i].toString().c_str());
            }
            printf("\n");
            return Value();
        },
        -1);
    defineGlobal("println", Value(printlnFn));
}

void VM::reportError(const std::string& message) {
    error = true;
    errorMessage = message;
    fprintf(stderr, "Error: %s\n", message.c_str());
}

bool VM::interpret() {
    error = false;
    
    Function* function = callStack.top().function;
    Chunk* chunk = &function->chunk;
    
    while (!error) {
        if (callStack.top().ip >= static_cast<int>(chunk->code.size())) {
            pop();  // Pop return value
            if (callStack.size() == 1) {
                lastValue = peek(0);
                return true;
            }
            callStack.pop();
            if (callStack.empty()) {
                return true;
            }
            function = callStack.top().function;
            chunk = &function->chunk;
        }
        
        int offset = callStack.top().ip;
        Opcode op = static_cast<Opcode>(chunk->code[offset++]);
        callStack.top().ip = offset;
        
        switch (op) {
            case Opcode::OP_CONST_NULL:
                push(Value(nullptr));
                break;
                
            case Opcode::OP_CONST_TRUE:
                push(true);
                break;
                
            case Opcode::OP_CONST_FALSE:
                push(false);
                break;
                
            case Opcode::OP_CONST_NUMBER: {
                double value = chunk->constants[chunk->code[offset++]].toNumber();
                callStack.top().ip = offset;
                push(value);
                break;
            }
            
            case Opcode::OP_CONST_STRING: {
                std::string value = chunk->constants[chunk->code[offset++]].toString();
                callStack.top().ip = offset;
                push(value);
                break;
            }
            
            case Opcode::OP_CONST_UNDEFINED:
                push(Value());
                break;
                
            case Opcode::OP_GET_LOCAL: {
                int slot = chunk->code[offset++];
                callStack.top().ip = offset;
                // Get local from stack frame
                int stackPos = callStack.top().stackStart + slot;
                if (stackPos >= 0 && stackPos < static_cast<int>(stack.size())) {
                    push(stack[stackPos]);
                }
                break;
            }
            
            case Opcode::OP_SET_LOCAL: {
                int slot = chunk->code[offset++];
                callStack.top().ip = offset;
                Value value = pop();
                int stackPos = callStack.top().stackStart + slot;
                if (stackPos >= 0 && stackPos < static_cast<int>(stack.size())) {
                    stack[stackPos] = value;
                } else if (stackPos >= 0 && stackPos == static_cast<int>(stack.size())) {
                    stack.push_back(value);
                }
                break;
            }
            
            case Opcode::OP_GET_GLOBAL: {
                std::string name = chunk->constants[chunk->code[offset++]].toString();
                callStack.top().ip = offset;
                push(getGlobal(name));
                break;
            }
            
            case Opcode::OP_SET_GLOBAL: {
                Value value = pop();
                std::string name = chunk->constants[chunk->code[offset++]].toString();
                callStack.top().ip = offset;
                setGlobal(name, value);
                break;
            }
            
            case Opcode::OP_ADD: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(a.toNumber() + b.toNumber());
                } else if (a.isString() && b.isString()) {
                    std::string result = a.toString() + b.toString();
                    push(result);
                } else {
                    reportError("Operands must be two numbers or two strings.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_SUBTRACT: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(a.toNumber() - b.toNumber());
                } else {
                    reportError("Operands must be numbers.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_MULTIPLY: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(a.toNumber() * b.toNumber());
                } else {
                    reportError("Operands must be numbers.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_DIVIDE: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(a.toNumber() / b.toNumber());
                } else {
                    reportError("Operands must be numbers.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_MODULO: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(fmod(a.toNumber(), b.toNumber()));
                } else {
                    reportError("Operands must be numbers.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_NEGATE: {
                Value a = pop();
                if (a.isNumber()) {
                    push(-a.toNumber());
                } else {
                    reportError("Operand must be a number.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(a == b);
                break;
            }
            
            case Opcode::OP_NOT_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(a != b);
                break;
            }
            
            case Opcode::OP_LESS: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(a.toNumber() < b.toNumber());
                } else {
                    reportError("Operands must be numbers.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_LESS_EQUAL: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(a.toNumber() <= b.toNumber());
                } else {
                    reportError("Operands must be numbers.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_GREATER: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(a.toNumber() > b.toNumber());
                } else {
                    reportError("Operands must be numbers.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_GREATER_EQUAL: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(a.toNumber() >= b.toNumber());
                } else {
                    reportError("Operands must be numbers.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_NOT: {
                push(!pop().toBool());
                break;
            }
            
            case Opcode::OP_AND: {
                Value b = peek(0);
                if (!b.toBool()) {
                    pop();
                    push(b);
                } else {
                    // TODO: Continue with right side
                    pop();
                }
                break;
            }
            
            case Opcode::OP_OR: {
                Value a = peek(0);
                if (a.toBool()) {
                    // TODO: Skip right side
                } else {
                    pop();
                }
                break;
            }
            
            case Opcode::OP_POP:
                pop();
                break;
                
            case Opcode::OP_JUMP: {
                int offset = chunk->code[callStack.top().ip++];
                callStack.top().ip += offset;
                break;
            }
            
            case Opcode::OP_JUMP_IF_FALSE: {
                int offset = chunk->code[callStack.top().ip++];
                Value condition = pop();
                if (!condition.toBool()) {
                    callStack.top().ip += offset;
                }
                break;
            }
            
            case Opcode::OP_JUMP_IF_TRUE: {
                int offset = chunk->code[callStack.top().ip++];
                Value condition = pop();
                if (condition.toBool()) {
                    callStack.top().ip += offset;
                }
                break;
            }
            
            case Opcode::OP_LOOP: {
                int offset = chunk->code[callStack.top().ip++];
                callStack.top().ip -= offset;
                break;
            }
            
            case Opcode::OP_CALL: {
                int argCount = chunk->code[callStack.top().ip++];
                Value callee = peek(argCount);
                
               if (callee.isObject() && callee.asObject()) {
                     if (callee.asObject()->type == ObjectType::Function) {
                         auto* func = reinterpret_cast<Function*>(callee.asObject());
                         
                         // Set up call frame
                         CallFrame frame;
                         frame.function = func;
                         frame.chunk = &func->chunk;
                         frame.ip = 0;
                         frame.stackStart = static_cast<int>(stack.size()) - argCount - 1;
                         
                         callStack.push(frame);
                     } else if (callee.asObject()->type == ObjectType::Native) {
                         auto* native = reinterpret_cast<NativeFunction*>(callee.asObject());
                         // Call native function
                         Value* args = &stack[stack.size() - argCount];
                         Value result = native->function(args, argCount);
                         
                         // Pop arguments and function
                         for (int i = 0; i <= argCount; i++) {
                             pop();
                         }
                         push(result);
                     } else {
                         reportError("Can only call functions.");
                         return false;
                     }
                 } else {
                     reportError("Can only call functions.");
                     return false;
                 }
                break;
            }
            
            case Opcode::OP_RETURN: {
                Value result;
                if (!stack.empty()) {
                    result = pop();
                }
                int returnPos = callStack.top().stackStart;
                
                // Pop the frame
                callStack.pop();
                
                if (callStack.empty()) {
                    lastValue = result;
                    return true;
                }
                
                // Push return value onto caller's stack
                stack[returnPos] = result;
                
                // Move back to caller's function
                function = callStack.top().function;
                chunk = &function->chunk;
                break;
            }
            
            case Opcode::OP_NEW_ARRAY: {
                int count = chunk->code[callStack.top().ip++];
                auto* arr = new ArrayObj();
                
                // Pop elements in reverse order
                for (int i = count - 1; i >= 0; i--) {
                    arr->elements.insert(arr->elements.begin(), pop());
                }
                
                push(Value(arr));
                break;
            }
            
            case Opcode::OP_NEW_OBJECT: {
                int count = chunk->code[callStack.top().ip++];
                auto* obj = new MapObj();
                
                // Pop pairs (value, key)
                for (int i = 0; i < count; i++) {
                    Value value = pop();
                    std::string key = pop().toString();
                    obj->entries[key] = value;
                }
                
                push(Value(obj));
                break;
            }
            
            case Opcode::OP_GET_PROPERTY: {
                std::string name = pop().toString();
                Value obj = pop();
                
                if (obj.isObject() && obj.asObject() && obj.asObject()->type == ObjectType::Map) {
                    auto* mapObj = reinterpret_cast<MapObj*>(obj.asObject());
                    auto it = mapObj->entries.find(name);
                    if (it != mapObj->entries.end()) {
                        push(it->second);
                    } else {
                        push(Value());  // undefined
                    }
                } else {
                    reportError("Object has no property: " + name);
                    return false;
                }
                break;
            }
            
            case Opcode::OP_SET_PROPERTY: {
                Value value = pop();
                std::string name = pop().toString();
                Value obj = pop();
                
                if (obj.isObject() && obj.asObject() && obj.asObject()->type == ObjectType::Map) {
                    auto* mapObj = reinterpret_cast<MapObj*>(obj.asObject());
                    mapObj->entries[name] = value;
                } else {
                    reportError("Object doesn't support property assignment.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_GET_INDEX: {
                Value indexVal = pop();
                Value arr = pop();
                
                if (arr.isObject() && arr.asObject() && arr.asObject()->type == ObjectType::Array) {
                    auto* arrayObj = reinterpret_cast<ArrayObj*>(arr.asObject());
                    int index = static_cast<int>(indexVal.toNumber());
                    if (index >= 0 && index < arrayObj->length()) {
                        push(arrayObj->at(index));
                    } else {
                        push(Value());  // undefined
                    }
                } else {
                    reportError("Can only index arrays.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_SET_INDEX: {
                Value value = pop();
                Value indexVal = pop();
                Value arr = pop();
                
                if (arr.isObject() && arr.asObject() && arr.asObject()->type == ObjectType::Array) {
                    auto* arrayObj = reinterpret_cast<ArrayObj*>(arr.asObject());
                    int index = static_cast<int>(indexVal.toNumber());
                    if (index >= 0 && index < arrayObj->length()) {
                        arrayObj->at(index) = value;
                    } else {
                        reportError("Array index out of bounds.");
                        return false;
                    }
                } else {
                    reportError("Can only index arrays.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_NEW_CLASS: {
                std::string name = chunk->constants[chunk->code[callStack.top().ip++]].toString();
                auto* klass = new Function(name, Chunk(), 0, false, false);
                push(Value(klass));
                break;
            }
            
            case Opcode::OP_METHOD: {
                std::string name = chunk->constants[chunk->code[callStack.top().ip++]].toString();
                // Method handling would go here
                pop();  // For now, just pop the method
                break;
            }
            
            case Opcode::OP_NOP:
                break;
                
            case Opcode::OP_END:
                return true;
                
            default:
                reportError("Unknown opcode: " + std::to_string(static_cast<int>(op)));
                return false;
        }
    }
    
    return false;
}

void VM::run(Function* function) {
    if (!function) {
        reportError("No function to run.");
        return;
    }
    
    CallFrame frame;
    frame.function = function;
    frame.chunk = &function->chunk;
    frame.ip = 0;
    frame.stackStart = static_cast<int>(stack.size());
    
    callStack.push(frame);
    interpret();
    
    if (!error) {
        printf("Result: %s\n", lastValue.toString().c_str());
    }
}

void VM::runChunk(Chunk& chunk) {
    auto* func = new Function("<script>", chunk, 0, true, false);
    
    run(func);
    delete func;
}

} // namespace hs
