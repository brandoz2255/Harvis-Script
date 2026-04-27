#include "vm.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../compiler/compiler.h"

namespace hs {

VM::VM() : error(false), panicActive(false), recoverValue(Value()) {
    stack.reserve(65535);
    registerNatives();
}

VM::~VM() {
    // Value destructors handle release() on all objects
    globals.clear();
    stack.clear();
    deferredCallStack.clear();
    
    // Clean up closed upvalue Values (destructor calls release on contained objects)
    for (auto& upvalue : upvalues) {
        if (upvalue && upvalue->closed) {
            delete upvalue->closed;
        }
    }
    upvalues.clear();
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
    if (stack.empty() || offset < 0 || offset >= static_cast<int>(stack.size())) {
        return Value();
    }
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
        globals[name] = value;
    } else {
        it->second = value;
    }
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
    
    interpret_loop:
    while (!error) {
        std::unique_lock<std::recursive_mutex> lock(vmMutex);
        if (panicActive) {
            panicActive = false;
            lock.unlock();
            continue;
        }
        
        Function* function = callStack.top().function;
        Chunk* chunk = &function->chunk;
        if (callStack.top().ip >= static_cast<int>(chunk->code.size())) {
            if (!stack.empty()) {
                pop();  // Pop return value
            }
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
                // Get local from locals array
                auto& locals = callStack.top().locals;
                if (slot >= 0 && slot < static_cast<int>(locals.size())) {
                    push(locals[slot]);
                }
                break;
            }
            
           case Opcode::OP_SET_LOCAL: {
                 int slot = chunk->code[offset++];
                 callStack.top().ip = offset;
                 Value value = pop();
                 auto& locals = callStack.top().locals;
                 while (static_cast<int>(locals.size()) <= slot) {
                     locals.push_back(Value());
                 }
                 locals[slot] = value;
                 break;
             }
            
case Opcode::OP_GET_GLOBAL: {
                 int index = chunk->code[offset++];
                 callStack.top().ip = offset;
                 std::string name = chunk->constants[index].toString();
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
                Value b = pop();
                Value a = pop();
                push(a.toBool() && b.toBool());
                break;
            }
            
            case Opcode::OP_OR: {
                Value b = pop();
                Value a = pop();
                push(a.toBool() || b.toBool());
                break;
            }
            
            case Opcode::OP_BITWISE_AND: {
                Value b = pop();
                Value a = pop();
                push(static_cast<double>(static_cast<int>(a.toNumber()) & static_cast<int>(b.toNumber())));
                break;
            }
            
            case Opcode::OP_BITWISE_OR: {
                Value b = pop();
                Value a = pop();
                push(static_cast<double>(static_cast<int>(a.toNumber()) | static_cast<int>(b.toNumber())));
                break;
            }
            
            case Opcode::OP_BITWISE_XOR: {
                Value b = pop();
                Value a = pop();
                push(static_cast<double>(static_cast<int>(a.toNumber()) ^ static_cast<int>(b.toNumber())));
                break;
            }
            
            case Opcode::OP_BITWISE_NOT: {
                Value a = pop();
                push(static_cast<double>(~static_cast<int>(a.toNumber())));
                break;
            }
            
            case Opcode::OP_SHIFT_LEFT: {
                Value b = pop();
                Value a = pop();
                push(static_cast<double>(static_cast<int>(a.toNumber()) << static_cast<int>(b.toNumber())));
                break;
            }
            
            case Opcode::OP_SHIFT_RIGHT: {
                Value b = pop();
                Value a = pop();
                push(static_cast<double>(static_cast<int>(a.toNumber()) >> static_cast<int>(b.toNumber())));
                break;
            }
            
        case Opcode::OP_POP:
                 if (stack.empty()) {
                     reportError("Stack underflow in OP_POP.");
                     return false;
                 }
                 pop();
                 break;
                
            case Opcode::OP_DUP: {
                Value top = peek(0);
                push(top);
                break;
            }
                
            case Opcode::OP_JUMP: {
                int offset = chunk->code[callStack.top().ip++];
                offset |= chunk->code[callStack.top().ip++] << 8;
                if (offset & 0x8000) offset -= 0x10000;
                callStack.top().ip += offset;
                break;
            }
            
       case Opcode::OP_JUMP_IF_FALSE: {
                 int offset = chunk->code[callStack.top().ip++];
                 offset |= chunk->code[callStack.top().ip++] << 8;
                 if (offset & 0x8000) offset -= 0x10000;
                 Value condition = pop();
                 if (!condition.toBool()) {
                     callStack.top().ip += offset;
                 }
                 break;
             }
            
            case Opcode::OP_JUMP_IF_TRUE: {
                int offset = chunk->code[callStack.top().ip++];
                offset |= chunk->code[callStack.top().ip++] << 8;
                if (offset & 0x8000) offset -= 0x10000;
                Value condition = pop();
                if (condition.toBool()) {
                    callStack.top().ip += offset;
                }
                break;
            }
            
            case Opcode::OP_LOOP: {
                 int offset = chunk->code[callStack.top().ip++];
                 offset |= chunk->code[callStack.top().ip++] << 8;
                 if (offset & 0x8000) offset -= 0x10000;
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
                  frame.stackStart = static_cast<int>(stack.size()) - argCount;
                  
                  // Initialize locals with function parameters
                  int arity = func->arity;
                  for (int i = 0; i < arity && i < argCount; i++) {
                      frame.locals.push_back(stack[frame.stackStart + i]);
                  }
                  while (frame.locals.size() < static_cast<size_t>(arity)) {
                      frame.locals.push_back(Value());
                  }
                  
                  callStack.push(frame);
                  break;
              } else if (callee.asObject()->type == ObjectType::Closure) {
                          auto* closure = reinterpret_cast<Closure*>(callee.asObject());
                          
                          CallFrame frame;
                          frame.function = closure->function;
                          frame.chunk = &closure->function->chunk;
                          frame.ip = 0;
                          frame.stackStart = static_cast<int>(stack.size()) - argCount;
                          
                          // Initialize locals with function parameters
                          int arity = closure->function->arity;
                          for (int i = 0; i < arity && i < argCount; i++) {
                              frame.locals.push_back(stack[frame.stackStart + i]);
                          }
                          while (frame.locals.size() < static_cast<size_t>(arity)) {
                              frame.locals.push_back(Value());
                          }
                          
                          callStack.push(frame);
                          break;
                     } else if (callee.asObject()->type == ObjectType::BoundMethod) {
                          auto* bound = reinterpret_cast<BoundMethod*>(callee.asObject());
                          Instance* instance = bound->instance;
                          
                          CallFrame frame;
                          frame.function = bound->method->function;
                          frame.chunk = &bound->method->function->chunk;
                          frame.ip = 0;
                          frame.stackStart = static_cast<int>(stack.size()) - argCount;
                          
                          // Initialize locals with 'this' as first param and function parameters
                          // Push instance as implicit first argument (this)
                          int arity = bound->method->function->arity;
                          // slot 0 = this
                          frame.locals.push_back(Value(instance));
                          // Remaining slots from stack
                          for (int i = 1; i < arity && i < argCount; i++) {
                              frame.locals.push_back(stack[frame.stackStart + i]);
                          }
                          while (frame.locals.size() < static_cast<size_t>(arity)) {
                              frame.locals.push_back(Value());
                          }
                          
                          callStack.push(frame);
                          break;
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
                int returnPos = callStack.top().stackStart - 1;
                int argCount = callStack.top().function->arity;

                // Pop the frame
                callStack.pop();

                if (callStack.empty()) {
                    lastValue = result;
                    return true;
                }

                // Pop arguments and callee
                for (int i = 0; i < argCount; i++) {
                    pop();
                }
                pop();  // Pop callee
                push(result);

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
                 
                 if (obj.isObject() && obj.asObject()) {
                     if (obj.asObject()->type == ObjectType::Map) {
                         auto* mapObj = reinterpret_cast<MapObj*>(obj.asObject());
                         auto it = mapObj->entries.find(name);
                         if (it != mapObj->entries.end()) {
                             push(it->second);
                         } else {
                             push(Value());  // undefined
                         }
                     } else if (obj.asObject()->type == ObjectType::Instance) {
                         auto* instance = reinterpret_cast<Instance*>(obj.asObject());
                         
                         // First check fields
                         auto fieldIt = instance->fields.find(name);
                         if (fieldIt != instance->fields.end()) {
                             push(fieldIt->second);
                         } else {
                             // Check methods in class
                             auto methodIt = instance->klass->methods.find(name);
                             if (methodIt != instance->klass->methods.end()) {
                                 auto* bound = new BoundMethod(instance, methodIt->second);
                                 push(Value(bound));
                             } else {
                                 push(Value());  // undefined
                             }
                         }
                     } else {
                         reportError("Object has no property: " + name);
                         return false;
                     }
                 } else {
                     reportError("Cannot get property of non-object: " + name);
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
                auto* klass = new ClassObj(name);
                push(Value(klass));
                break;
            }
            
            case Opcode::OP_NEW_INSTANCE: {
                // Pop class object, create new instance
                Value classVal = pop();
                if (classVal.isObject() && classVal.asObject() && classVal.asObject()->type == ObjectType::Class) {
                    auto* klass = reinterpret_cast<ClassObj*>(classVal.asObject());
                    auto* instance = new Instance(klass);
                    push(Value(instance));
                } else {
                    reportError("Cannot create instance from non-class.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_METHOD: {
                std::string name = chunk->constants[chunk->code[callStack.top().ip++]].toString();
                Value methodVal = pop();
                
                // Pop the class (should be on stack below the method)
                if (stack.size() >= 2) {
                    Value classVal = stack[stack.size() - 2];
                    if (classVal.isObject() && classVal.asObject() && classVal.asObject()->type == ObjectType::Class) {
                        auto* klass = reinterpret_cast<ClassObj*>(classVal.asObject());
                        if (methodVal.isObject() && methodVal.asObject() && methodVal.asObject()->type == ObjectType::Closure) {
                            auto* closure = reinterpret_cast<Closure*>(methodVal.asObject());
                            klass->methods[name] = closure;
                            closure->retain();  // Keep method alive
                        }
                    }
                }
                // Remove class from stack, leave method on stack (for assignment)
                break;
            }
            
            case Opcode::OP_CONST_FUNCTION: {
                int funcIndex = chunk->code[callStack.top().ip++];
                Value funcVal = chunk->constants[funcIndex];
                Function* func = static_cast<Function*>(funcVal.asObject());
                push(Value(func));
                break;
            }
            
            case Opcode::OP_CLOSURE: {
                int funcIndex = chunk->code[callStack.top().ip++];
                int upvalueCount = chunk->code[callStack.top().ip++];
                
                Value funcVal = chunk->constants[funcIndex];
                Function* func = static_cast<Function*>(funcVal.asObject());
                
                std::vector<std::shared_ptr<Upvalue>> closureUpvalues;
                closureUpvalues.reserve(upvalueCount);
                
                for (int j = 0; j < upvalueCount; j++) {
                    int flags = chunk->code[callStack.top().ip++];
                    bool isUpvalue = (flags >> 7) & 1;
                    int index = flags & 0x7F;
                    
                    if (isUpvalue) {
                        closureUpvalues.push_back(upvalues[index]);
                    } else {
                        closureUpvalues.push_back(captureUpvalue(callStack.top().stackStart + index));
                    }
                }
                
                auto* closure = new Closure(func, std::move(closureUpvalues));
                push(Value(closure));
                break;
            }
            
            case Opcode::OP_CLOSE_UPVALUE: {
                int slot = chunk->code[callStack.top().ip++];
                callStack.top().ip++;
                closeUpvalues(slot);
                break;
            }
            
            case Opcode::OP_NEW_STRUCT: {
                int nameIndex = chunk->code[callStack.top().ip++];
                int fieldCount = chunk->code[callStack.top().ip++];
                
                std::string structName = chunk->constants[nameIndex].toString();
                auto* structObj = new MapObj();
                
                // Read field names (already popped from stack by the emit order)
                std::vector<int> fieldNameIndices;
                for (int i = 0; i < fieldCount; i++) {
                    fieldNameIndices.push_back(chunk->code[callStack.top().ip++]);
                }
                
                for (int i = fieldCount - 1; i >= 0; i--) {
                    Value value = pop();
                    std::string fieldName = chunk->constants[fieldNameIndices[i]].toString();
                    structObj->entries[fieldName] = value;
                }
                
                push(Value(structObj));
                break;
            }
            
            case Opcode::OP_GET_FIELD: {
                int nameIndex = chunk->code[callStack.top().ip++];
                
                std::string fieldName = chunk->constants[nameIndex].toString();
                Value obj = pop();
                
                if (obj.isObject() && obj.asObject() && obj.asObject()->type == ObjectType::Map) {
                    auto* mapObj = reinterpret_cast<MapObj*>(obj.asObject());
                    auto it = mapObj->entries.find(fieldName);
                    if (it != mapObj->entries.end()) {
                        push(it->second);
                    } else {
                        push(Value());
                    }
                } else {
                    reportError("Value is not a struct.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_GET_FIELD_OPTIONAL: {
                int nameIndex = chunk->code[callStack.top().ip++];
                
                std::string fieldName = chunk->constants[nameIndex].toString();
                Value obj = pop();
                
                if (obj.isNull() || obj.isUndefined()) {
                    push(Value());
                } else if (obj.isObject() && obj.asObject() && obj.asObject()->type == ObjectType::Map) {
                    auto* mapObj = reinterpret_cast<MapObj*>(obj.asObject());
                    auto it = mapObj->entries.find(fieldName);
                    if (it != mapObj->entries.end()) {
                        push(it->second);
                    } else {
                        push(Value());
                    }
                } else {
                    push(Value());
                }
                break;
            }
            
            case Opcode::OP_GET_INDEX_OPTIONAL: {
                Value obj = pop();
                Value index = pop();
                
                if (obj.isNull() || obj.isUndefined()) {
                    push(Value());
                } else if (obj.isObject() && obj.asObject() && obj.asObject()->type == ObjectType::Array) {
                    auto* arrayObj = reinterpret_cast<ArrayObj*>(obj.asObject());
                    int idx = static_cast<int>(index.toNumber());
                    if (idx >= 0 && idx < static_cast<int>(arrayObj->elements.size())) {
                        push(arrayObj->elements[idx]);
                    } else {
                        push(Value());
                    }
                } else {
                    push(Value());
                }
                break;
            }
            
            case Opcode::OP_NULLISH_COALESCE: {
                // Stack: left, right
                // If left is null/undefined, result is right; otherwise result is left
                Value right = pop();
                Value left = pop();
                if (left.isNull() || left.isUndefined()) {
                    push(right);
                } else {
                    push(left);
                }
                break;
            }
            
            case Opcode::OP_LEN: {
                Value collection = pop();
                int length = 0;
                if (collection.isString()) {
                    length = static_cast<int>(collection.toString().size());
                } else if (collection.isObject() && collection.asObject()) {
                    auto* obj = collection.asObject();
                    if (obj->type == ObjectType::Array) {
                        auto* arr = reinterpret_cast<ArrayObj*>(obj);
                        length = arr->length();
                    } else if (obj->type == ObjectType::Map) {
                        auto* mapObj = reinterpret_cast<MapObj*>(obj);
                        length = static_cast<int>(mapObj->entries.size());
                    }
                }
                push(static_cast<double>(length));
                break;
            }
            
     case Opcode::OP_MAKE: {
                  // Stack: typeName, capacity (typeName pushed first, so on bottom)
                  // Pop capacity first (top of stack), then typeName
                  std::string typeName = "unknown";
                  int capacity = 0;
                  
                  // Always expect at least 2 args: type name + capacity
                  // For make("map") with no capacity, capacity stays 0
                  if (stack.size() >= 2) {
                      capacity = static_cast<int>(pop().toNumber());
                  }
                  if (stack.size() >= 1) {
                      typeName = pop().toString();
                  }
                 
                 // Create object based on type
                 if (typeName == "array" || typeName == "Array") {
                     auto* arr = new ArrayObj();
                     for (int i = 0; i < capacity; i++) {
                         arr->elements.push_back(Value());
                     }
                     push(Value(arr, Type::array()));
                 } else if (typeName == "map" || typeName == "Map") {
                     push(Value(new MapObj(), Type::object()));
                 } else if (typeName == "channel" || typeName == "Channel") {
                     push(Value(new ChannelObj(capacity), Type::channel()));
                 } else {
                     reportError("make() requires a valid type: array, map, or channel.");
                     return false;
                 }
                 break;
             }
            
            case Opcode::OP_APPEND: {
                // Stack: element, array -> new array
                Value element = pop();
                Value arrVal = pop();
                
                if (arrVal.isObject() && arrVal.asObject() && arrVal.asObject()->type == ObjectType::Array) {
                    auto* arr = reinterpret_cast<ArrayObj*>(arrVal.asObject());
                    auto* newArr = new ArrayObj();
                    newArr->elements = arr->elements;
                    newArr->elements.push_back(element);
                    push(Value(newArr, Type::array()));
                } else {
                    reportError("append() requires an array.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_COPY: {
                // Stack: dest, src -> count
                Value srcVal = pop();
                Value destVal = pop();
                
                int count = 0;
                if (destVal.isObject() && destVal.asObject() && destVal.asObject()->type == ObjectType::Array) {
                    if (srcVal.isObject() && srcVal.asObject() && srcVal.asObject()->type == ObjectType::Array) {
                        auto* dest = reinterpret_cast<ArrayObj*>(destVal.asObject());
                        auto* src = reinterpret_cast<ArrayObj*>(srcVal.asObject());
                        count = std::min(dest->length(), src->length());
                        for (int i = 0; i < count; i++) {
                            dest->elements[i] = src->elements[i];
                        }
                    }
                }
                push(static_cast<double>(count));
                break;
            }
            
            case Opcode::OP_DELETE: {
                // Stack: key, map -> success
                Value key = pop();
                Value mapVal = pop();
                
                bool success = false;
                if (mapVal.isObject() && mapVal.asObject() && mapVal.asObject()->type == ObjectType::Map) {
                    auto* mapObj = reinterpret_cast<MapObj*>(mapVal.asObject());
                    std::string keyStr = key.toString();
                    success = mapObj->entries.erase(keyStr) > 0;
                }
                push(success);
                break;
            }
            
           case Opcode::OP_IMPORT: {
                // Stack: moduleName -> module
                std::string moduleName = pop().toString();
                Value module = loadModule(moduleName);
                push(module);
                break;
            }
            
            case Opcode::OP_EXPORT: {
                uint8_t nameIdx = chunk->code[callStack.top().ip++];
                std::string exportName = chunk->constants[nameIdx].toString();
                
                Value exportValue = peek(0);
                currentModule->entries[exportName] = exportValue;
                break;
            }

            case Opcode::OP_TRY: {
                int catchOffset = chunk->code[callStack.top().ip] | (chunk->code[callStack.top().ip + 1] << 8);
                callStack.top().ip += 2;
                TryFrame tf;
                tf.tryBodyStart = callStack.top().ip;
                tf.catchIP = tf.tryBodyStart + catchOffset;
                tf.stackDepth = static_cast<int>(stack.size());
                tf.callDepth = static_cast<int>(callStack.size());
                tryStack.push_back(tf);
                break;
            }

            case Opcode::OP_CATCH: {
                uint8_t varIdx = chunk->code[callStack.top().ip++];
                std::string varName = chunk->constants[varIdx].toString();
                // Error value is on stack (pushed by OP_THROW)
                Value errorValue = stack.back();
                stack.pop_back();
                setGlobal(varName, errorValue);
                // Pop try frame
                if (!tryStack.empty()) {
                    tryStack.pop_back();
                }
                break;
            }

            case Opcode::OP_THROW: {
                Value errorValue = pop();
                // Walk tryStack from top (most recent) looking for a handler
                for (int i = static_cast<int>(tryStack.size()) - 1; i >= 0; i--) {
                    auto& tf = tryStack[i];
                    // Check if throw is within this try block's scope:
                    // - If same call frame: IP must be within try body range
                    // - If deeper call (from nested function call): always in range
                    bool inRange = false;
                    if (static_cast<int>(callStack.size()) == tf.callDepth) {
                        // Same frame - check IP range
                        inRange = (callStack.top().ip >= tf.tryBodyStart) &&
                                  (callStack.top().ip < tf.catchIP);
                    } else {
                        // Deeper frame (nested call from within try) - always catchable
                        inRange = true;
                    }
                    if (inRange) {
                        // Found handler - unwind to catch
                        // Restore stack to try start depth
                        while (static_cast<int>(stack.size()) > tf.stackDepth) {
                            stack.pop_back();
                        }
                        // Push error value for catch to consume
                        push(errorValue);
                        // Unwind call frames if we went deeper
                        while (static_cast<int>(callStack.size()) > tf.callDepth) {
                            callStack.pop();
                        }
                        // Set IP to catch block
                        callStack.top().ip = tf.catchIP;
                        // Remove this and all inner try frames
                        tryStack.resize(i);
                        goto interpret_loop;
                    }
                }
                // No handler found - abort with error
                executeDeferred();
                reportError(errorValue.toString());
                return false;
            }
            
            case Opcode::OP_RANGE_ARRAY: {
                // Stack: array -> iterator
                // Iterator is represented as a special object with array reference and index
                Value arrVal = pop();
                
                if (arrVal.isObject() && arrVal.asObject() && arrVal.asObject()->type == ObjectType::Array) {
                    auto* arr = reinterpret_cast<ArrayObj*>(arrVal.asObject());
                    // Create iterator object (we'll use a special marker)
                    // For simplicity, push array and index as separate values
                    // Actually, let's push a special iterator object
                    // For now, just push the array and a special iterator marker
                    push(static_cast<double>(0)); // index
                    push(arrVal); // array
                } else {
                    push(Value());
                    push(Value());
                }
                break;
            }
            
            case Opcode::OP_RANGE_NEXT: {
                // Stack: index, array -> value, newIndex, hasNext
                Value arrVal = pop();
                Value indexVal = pop();
                
                int index = static_cast<int>(indexVal.toNumber());
                bool hasNext = false;
                Value value = Value();
                int newIndex = index + 1;
                
                if (arrVal.isObject() && arrVal.asObject() && arrVal.asObject()->type == ObjectType::Array) {
                    auto* arr = reinterpret_cast<ArrayObj*>(arrVal.asObject());
                    if (index < arr->length()) {
                        hasNext = true;
                        value = arr->elements[index];
                    }
                }
                
                push(hasNext);
                push(static_cast<double>(newIndex));
                push(value);
                break;
            }
            
            case Opcode::OP_RANGE_STRING: {
                // Stack: string -> iterator
                Value strVal = pop();
                if (strVal.isString()) {
                    push(static_cast<double>(0)); // index
                    push(strVal); // string
                } else {
                    push(Value());
                    push(Value());
                }
                break;
            }
            
            case Opcode::OP_RANGE_MAP: {
                // Stack: map -> iterator
                Value mapVal = pop();
                if (mapVal.isObject() && mapVal.asObject() && mapVal.asObject()->type == ObjectType::Map) {
                    auto* mapObj = reinterpret_cast<MapObj*>(mapVal.asObject());
                    push(static_cast<double>(0)); // index
                    push(mapVal); // map
                } else {
                    push(Value());
                    push(Value());
                }
                break;
            }
            
            case Opcode::OP_RANGE_MAP_NEXT: {
                // Stack: index, map -> hasNext, key, value
                Value mapVal = pop();
                Value indexVal = pop();
                
                int index = static_cast<int>(indexVal.toNumber());
                bool hasNext = false;
                Value key = Value();
                Value value = Value();
                
                if (mapVal.isObject() && mapVal.asObject() && mapVal.asObject()->type == ObjectType::Map) {
                    auto* mapObj = reinterpret_cast<MapObj*>(mapVal.asObject());
                    if (index < static_cast<int>(mapObj->entries.size())) {
                        hasNext = true;
                        // Get nth entry
                        int i = 0;
                        for (auto& pair : mapObj->entries) {
                            if (i == index) {
                                key = Value(pair.first);
                                value = pair.second;
                                break;
                            }
                            i++;
                        }
                    }
                }
                
                push(value);
                push(key);
                push(hasNext);
                break;
            }
            
            case Opcode::OP_ARRAY_LENGTH: {
                // Stack: array -> length
                Value arrVal = pop();
                int length = 0;
                if (arrVal.isObject() && arrVal.asObject() && arrVal.asObject()->type == ObjectType::Array) {
                    auto* arr = reinterpret_cast<ArrayObj*>(arrVal.asObject());
                    length = arr->length();
                }
                push(static_cast<double>(length));
                break;
            }
            
            case Opcode::OP_ARRAY_AT: {
                // Stack: index, array -> value
                Value arrVal = pop();
                Value indexVal = pop();
                
                int index = static_cast<int>(indexVal.toNumber());
                Value value = Value();
                
                if (arrVal.isObject() && arrVal.asObject() && arrVal.asObject()->type == ObjectType::Array) {
                    auto* arr = reinterpret_cast<ArrayObj*>(arrVal.asObject());
                    if (index >= 0 && index < arr->length()) {
                        value = arr->elements[index];
                    }
                }
                
push(value);
                break;
            }
            
            case Opcode::OP_TYPE_CHECK: {
                // Stack: value, typeIdx -> bool
                int typeIdx = chunk->code[callStack.top().ip++];
                Value value = pop();
                
                // Get type name from constants
                std::string typeName = chunk->constants[typeIdx].toString();
                bool matches = false;
                
                if (value.isNumber()) {
                    matches = (typeName == "number" || typeName == "Number");
                } else if (value.isString()) {
                    matches = (typeName == "string" || typeName == "String");
                } else if (value.isBoolean()) {
                    matches = (typeName == "boolean" || typeName == "Boolean");
                } else if (value.isNull()) {
                    matches = (typeName == "null" || typeName == "Null");
                } else if (value.isObject()) {
                    if (value.asObject() && value.asObject()->type == ObjectType::Array) {
                        matches = (typeName == "array" || typeName == "Array");
                    } else if (value.asObject() && value.asObject()->type == ObjectType::Map) {
                        matches = (typeName == "map" || typeName == "Map");
                    } else if (value.asObject() && value.asObject()->type == ObjectType::Function) {
                        matches = (typeName == "function" || typeName == "Function");
                    }
                }
                
                push(matches);
                break;
            }

            case Opcode::OP_SET_FIELD: {
                int nameIndex = chunk->code[callStack.top().ip++];
                
                std::string fieldName = chunk->constants[nameIndex].toString();
                Value value = pop();
                Value obj = pop();
                
                if (obj.isObject() && obj.asObject() && obj.asObject()->type == ObjectType::Map) {
                    auto* mapObj = reinterpret_cast<MapObj*>(obj.asObject());
                    mapObj->entries[fieldName] = value;
                } else {
                    reportError("Value is not a struct.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_DEFER: {
                Value callable = pop();
                deferredCallStack.push_back({callable, {}});
                break;
            }
            
             case Opcode::OP_DEFERRED_RETURN: {
                bool programEnded = false;
                for (int i = static_cast<int>(deferredCallStack.size()) - 1; i >= 0; i--) {
                    auto& dc = deferredCallStack[i];
                    push(dc.callable);
                    int argCount = static_cast<int>(dc.args.size());
                    
                    if (dc.callable.isObject() && dc.callable.asObject()) {
                        auto* obj = dc.callable.asObject();
                        if (obj->type == ObjectType::Closure) {
                            auto* closure = reinterpret_cast<Closure*>(obj);
                            CallFrame frame;
                            frame.function = closure->function;
                            frame.chunk = &closure->function->chunk;
                            frame.ip = 0;
                            frame.stackStart = static_cast<int>(stack.size()) - argCount - 1;
                            callStack.push(frame);
                            programEnded = interpret();
                            if (!programEnded && !callStack.empty()) {
                                callStack.pop();
                            }
                        } else if (obj->type == ObjectType::Native) {
                            auto* native = reinterpret_cast<NativeFunction*>(obj);
                            Value* argPtr = dc.args.empty() ? nullptr : dc.args.data();
                            Value result = native->function(argPtr, argCount);
                            pop();  // Pop callable
                            push(result);
                        }
                    }
                }
                deferredCallStack.clear();
                if (programEnded) return true;
                break;
            }
            
            case Opcode::OP_PANIC: {
                panicValue = pop();
                panicActive = true;
                reportError("Panic: " + panicValue.toString());
                return false;
            }
            
            case Opcode::OP_RECOVER: {
                recoverValue = panicActive ? panicValue : Value();
                panicActive = false;
                panicValue = Value();
                push(recoverValue);
                break;
            }
            
            case Opcode::OP_IS_INSTANCE: {
                int nameIndex = chunk->code[callStack.top().ip++];
                
                std::string interfaceName = chunk->constants[nameIndex].toString();
                Value obj = pop();
                
                bool isInstance = true;
                if (obj.isObject() && obj.asObject()) {
                    auto* o = obj.asObject();
                    if (o->type != ObjectType::Map) {
                        isInstance = false;
                    }
                } else {
                    isInstance = false;
                }
                
                push(isInstance);
                break;
            }
            
            case Opcode::OP_INVOKE: {
                int nameIndex = chunk->code[callStack.top().ip++];
                
                std::string methodName = chunk->constants[nameIndex].toString();
                int argCount = chunk->code[callStack.top().ip++];
                
                Value receiver = peek(argCount);
                Value obj = pop();
                
                if (obj.isObject() && obj.asObject() && obj.asObject()->type == ObjectType::Map) {
                    auto* mapObj = reinterpret_cast<MapObj*>(obj.asObject());
                    auto it = mapObj->entries.find(methodName);
                    if (it != mapObj->entries.end()) {
                        pop();
                        // Push receiver as first arg
                        push(receiver);
                        // Call the method
                        Value callable = it->second;
                        push(callable);
                        // Inline OP_CALL logic
                        int callArgCount = argCount + 1;
                        Value callee = peek(callArgCount);
                        if (callee.isObject() && callee.asObject()) {
                            auto* obj2 = callee.asObject();
                            if (obj2->type == ObjectType::Closure) {
                                auto* closure = reinterpret_cast<Closure*>(obj2);
                                CallFrame frame;
                                frame.function = closure->function;
                                frame.chunk = &closure->function->chunk;
                                frame.ip = 0;
                                frame.stackStart = static_cast<int>(stack.size()) - callArgCount;
                                callStack.push(frame);
                            } else if (obj2->type == ObjectType::Native) {
                                auto* native = reinterpret_cast<NativeFunction*>(obj2);
                                Value* args = &stack[stack.size() - callArgCount];
                                Value result = native->function(args, callArgCount);
                                for (int i = 0; i <= callArgCount; i++) {
                                    pop();
                                }
                                push(result);
                            } else if (obj2->type == ObjectType::Function) {
                                auto* func = reinterpret_cast<Function*>(obj2);
                                CallFrame frame;
                                frame.function = func;
                                frame.chunk = &func->chunk;
                                frame.ip = 0;
                                frame.stackStart = static_cast<int>(stack.size()) - callArgCount;
                                callStack.push(frame);
                            }
        }
            }
                }
                break;
            }
            
            case Opcode::OP_MAKE_CHANNEL: {
                Value capacityVal = pop();
                int capacity = static_cast<int>(capacityVal.toNumber());
                if (capacity < 0) capacity = 0;
                auto* channel = new ChannelObj(capacity);
                push(Value(channel, Type::channel()));
                break;
            }
            
            case Opcode::OP_SEND: {
                Value value = pop();
                Value channelVal = pop();
                if (channelVal.isObject() && channelVal.asObject() && channelVal.asObject()->type == ObjectType::Channel) {
                    auto* channel = reinterpret_cast<ChannelObj*>(channelVal.asObject());
                    channel->send(value);
                } else {
                    reportError("Cannot send to non-channel.");
                    return false;
                }
                push(Value());  // Push undefined as result
                break;
            }
            
            case Opcode::OP_RECEIVE: {
                Value channelVal = pop();
                if (channelVal.isObject() && channelVal.asObject() && channelVal.asObject()->type == ObjectType::Channel) {
                    auto* channel = reinterpret_cast<ChannelObj*>(channelVal.asObject());
                    push(channel->receive());
                } else {
                    reportError("Cannot receive from non-channel.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_SELECT_CASE: {
                break;
            }

            case Opcode::OP_SELECT_DEFAULT: {
                break;
            }

            case Opcode::OP_SELECT_CASE_END: {
                break;
            }

        case Opcode::OP_SELECT: {
                int caseCount = chunk->code[callStack.top().ip++];

                struct SelectCase {
                    Closure* body;
                    ChannelObj* channel;
                    bool isSend;
                    Value sendValue;
                    bool isDefault;
                };

                std::vector<SelectCase> cases(caseCount);
                for (int i = caseCount - 1; i >= 0; i--) {
                    cases[i].sendValue = pop();
                    cases[i].isSend = pop().toBool();
                    Value chVal = pop();
                    cases[i].isDefault = !chVal.isObject() || !chVal.asObject() || chVal.asObject()->type != ObjectType::Channel;
                    cases[i].channel = cases[i].isDefault ? nullptr : reinterpret_cast<ChannelObj*>(chVal.asObject());
                    Value cv = pop();
                    cases[i].body = cv.isObject() && cv.asObject() && cv.asObject()->type == ObjectType::Closure
                        ? reinterpret_cast<Closure*>(cv.asObject()) : nullptr;
                }

                // Check ready channels
                std::vector<int> ready;
                for (int i = 0; i < caseCount; i++) {
                    auto& sc = cases[i];
                    if (sc.isDefault || !sc.channel) continue;
                    std::lock_guard<std::mutex> lock(sc.channel->mtx);
                    if (sc.isSend) {
                        if (sc.channel->closed || static_cast<int>(sc.channel->queue.size()) < sc.channel->capacity)
                            ready.push_back(i);
                    } else {
                        if (!sc.channel->queue.empty() || sc.channel->closed)
                            ready.push_back(i);
                    }
                }

                int selected = -1;
                if (!ready.empty()) selected = ready[rand() % ready.size()];
                else {
                    for (int i = 0; i < caseCount; i++) {
                        if (cases[i].isDefault) { selected = i; break; }
                    }
                }

                if (selected >= 0 && cases[selected].body) {
                    auto& sc = cases[selected];
                    if (!sc.isDefault && sc.channel) {
                        if (sc.isSend) sc.channel->send(sc.sendValue);
                        else sc.channel->receive();
                    }
                    // Execute closure body
                    push(Value(sc.body));
                    CallFrame df;
                    df.function = sc.body->function;
                    df.chunk = &sc.body->function->chunk;
                    df.ip = 0;
                    df.stackStart = static_cast<int>(stack.size()) - 1;
                    callStack.push(df);
                }
                break;
            }
            
            case Opcode::OP_MUTEX_NEW: {
                auto* mutex = new MutexObj();
                push(Value(mutex, Type::mutex()));
                break;
            }
            
            case Opcode::OP_MUTEX_LOCK: {
                Value mutexVal = pop();
                if (mutexVal.isObject() && mutexVal.asObject() && mutexVal.asObject()->type == ObjectType::Mutex) {
                    auto* mutex = reinterpret_cast<MutexObj*>(mutexVal.asObject());
                    mutex->lock();
                } else {
                    reportError("Value is not a mutex.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_MUTEX_UNLOCK: {
                Value mutexVal = pop();
                if (mutexVal.isObject() && mutexVal.asObject() && mutexVal.asObject()->type == ObjectType::Mutex) {
                    auto* mutex = reinterpret_cast<MutexObj*>(mutexVal.asObject());
                    mutex->unlock();
                } else {
                    reportError("Value is not a mutex.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_WAITGROUP_NEW: {
                auto* wg = new WaitGroupObj();
                push(Value(wg, Type::waitgroup()));
                break;
            }
            
            case Opcode::OP_WAITGROUP_ADD: {
                Value deltaVal = pop();
                Value wgVal = pop();
                int delta = static_cast<int>(deltaVal.toNumber());
                if (wgVal.isObject() && wgVal.asObject() && wgVal.asObject()->type == ObjectType::WaitGroup) {
                    auto* wg = reinterpret_cast<WaitGroupObj*>(wgVal.asObject());
                    wg->add(delta);
                } else {
                    reportError("Value is not a waitgroup.");
                    return false;
                }
                break;
            }
            
            case Opcode::OP_WAITGROUP_WAIT: {
                Value wgVal = pop();
                if (wgVal.isObject() && wgVal.asObject() && wgVal.asObject()->type == ObjectType::WaitGroup) {
                    auto* wg = reinterpret_cast<WaitGroupObj*>(wgVal.asObject());
                    wg->wait();
                } else {
                    reportError("Value is not a waitgroup.");
                    return false;
                }
                break;
            }
            
           case Opcode::OP_GO: {
                int argCount = chunk->code[callStack.top().ip++];
                callStack.top().ip++;
                
                Value callee = peek(argCount);
                
                if (callee.isObject() && callee.asObject()) {
                    auto* obj = callee.asObject();
                    if (obj->type == ObjectType::Closure) {
                        auto* closure = reinterpret_cast<Closure*>(obj);
                        CallFrame frame;
                        frame.function = closure->function;
                        frame.chunk = &closure->function->chunk;
                        frame.ip = 0;
                        frame.stackStart = static_cast<int>(stack.size()) - argCount - 1;
                        std::thread t([this, frame]() {
                            std::unique_lock<std::recursive_mutex> lock(vmMutex);
                            callStack.push(frame);
                            interpret();
                            lock.unlock();
                            if (!callStack.empty()) {
                                callStack.pop();
                            }
                        });
                        t.detach();
                    }
                }
               // Pop function + args
                for (int i = 0; i <= argCount; i++) pop();
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

void VM::executeDeferred() {
    for (int i = static_cast<int>(deferredCallStack.size()) - 1; i >= 0; i--) {
        auto& dc = deferredCallStack[i];
        push(dc.callable);
        Value calleeVal = peek(0);
        if (calleeVal.isObject() && calleeVal.asObject()) {
            auto* obj = calleeVal.asObject();
            if (obj->type == ObjectType::Closure) {
                auto* closure = reinterpret_cast<Closure*>(obj);
                CallFrame df;
                df.function = closure->function;
                df.chunk = &closure->function->chunk;
                df.ip = 0;
                df.stackStart = static_cast<int>(stack.size()) - 1;
                callStack.push(df);
                while (!error && callStack.top().ip < static_cast<int>(callStack.top().chunk->code.size())) {
                    interpret();
                }
            }
        }
    }
    deferredCallStack.clear();
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

Value VM::loadModule(const std::string& moduleName) {
    std::string resolvedPath = moduleName;
    
    if (!modules.count(resolvedPath)) {
        std::ifstream file(resolvedPath);
        if (!file.is_open()) {
            resolvedPath = "stdlib/" + moduleName;
        }
    }
    
    if (modules.count(resolvedPath)) {
        return modules[resolvedPath];
    }
    
    std::ifstream file(resolvedPath);
    if (!file.is_open()) {
        reportError("Cannot open module file: " + moduleName);
        return Value();
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();
    
    Lexer lexer(source, resolvedPath);
    auto tokens = lexer.tokenize();
    if (lexer.hasErrors()) {
        reportError("Lexer error in module: " + resolvedPath);
        return Value();
    }
    
    Parser parser(tokens);
    auto program = parser.parse();
    if (parser.hasErrors()) {
        reportError("Parser error in module: " + resolvedPath);
        return Value();
    }
    
    Compiler compiler;
    compiler.compile(program);
    if (compiler.hasErrors()) {
        reportError("Compiler error in module: " + resolvedPath);
        return Value();
    }
    
    Chunk* chunk = compiler.getChunk();
    auto* func = new Function(resolvedPath, *chunk, 0, true, false);
    
    VM moduleVm;
    auto* exports = new MapObj();
    moduleVm.currentModule = exports;
    moduleVm.run(func);
    delete func;
    
    if (moduleVm.hasError()) {
        reportError("Runtime error in module: " + resolvedPath + " - " + moduleVm.getError());
        return Value();
    }
    
    auto& modGlobals = moduleVm.getGlobals();
    for (auto& [name, val] : modGlobals) {
        if (val.isObject() && val.asObject()) {
            auto* obj = val.asObject();
            if (obj->type == ObjectType::Closure || obj->type == ObjectType::Function || obj->type == ObjectType::Native) {
                exports->entries[name] = val;
            }
        } else if (val.isNumber() || val.isString() || val.isBoolean() || val.isNull()) {
            exports->entries[name] = val;
        }
    }
    
    Value moduleVal(Value(exports, Type::of(resolvedPath)));
    modules[resolvedPath] = moduleVal;
    if (moduleName != resolvedPath) {
        modules[moduleName] = moduleVal;
    }
    return moduleVal;
}

} // namespace hs
