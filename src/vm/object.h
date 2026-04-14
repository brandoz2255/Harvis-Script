#ifndef HARVIS_VM_OBJECT_H
#define HARVIS_VM_OBJECT_H

#include "../core/types.h"
#include "../compiler/chunk.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace hs {

// Forward declaration
class Upvalue;

// Object types
enum class ObjectType {
    None,
    Function,
    Closure,
    Instance,
    Native,
    BoundMethod,
    Array,
    String,
    Map
};

// Base object class
class RuntimeObject {
public:
    ObjectType type;
    Type declaredType;
    
    RuntimeObject(ObjectType t, Type dt = Type::object()) : type(t), declaredType(dt) {}
    virtual ~RuntimeObject() = default;
    
    virtual std::string toString() const {
        return "[Object " + std::to_string(static_cast<int>(type)) + "]";
    }
};

// Upvalue for closures
class Upvalue {
public:
    Value* slot;
    Value* closed;
    bool isClosed;
    
    Upvalue(Value* s) : slot(s), closed(nullptr), isClosed(false) {}
    
    void close() {
        closed = new Value(*slot);
        isClosed = true;
    }
    
    ~Upvalue() {
        if (closed) delete closed;
    }
};

// Native function (C++ implementation)
class NativeFunction : public RuntimeObject {
public:
    std::string name;
    std::function<Value(Value*, int)> function;
    int arity;
    
    NativeFunction(std::string n, std::function<Value(Value*, int)> fn, int a)
        : RuntimeObject(ObjectType::Native, Type::function()), name(std::move(n)), 
          function(fn), arity(a) {}
    
    std::string toString() const override {
        return "[NativeFunction " + name + "]";
    }
};

// Compiled function (runtime)
class Function : public RuntimeObject {
public:
    std::string name;
    Chunk chunk;
    int arity;
    bool isModule;
    bool isClassMethod;
    
    Function(std::string n, Chunk c, int a, bool m = false, bool cm = false)
        : RuntimeObject(ObjectType::Function, Type::function()), name(std::move(n)),
          chunk(std::move(c)), arity(a), isModule(m), isClassMethod(cm) {}
    
    std::string toString() const override {
        return "[Function " + name + "]";
    }
};

// Closure (function + captured upvalues)
class Closure : public RuntimeObject {
public:
    Function* function;
    std::vector<std::shared_ptr<Upvalue>> upvalues;
    
    Closure(Function* fn, std::vector<std::shared_ptr<Upvalue>> ups)
        : RuntimeObject(ObjectType::Closure, Type::function()), function(fn), 
          upvalues(std::move(ups)) {}
    
    std::string toString() const override {
        return "[Closure " + function->name + "]";
    }
};

// Class instance
class Instance : public RuntimeObject {
public:
    Function* klass;
    std::unordered_map<std::string, Value> fields;
    
    Instance(Function* k) : RuntimeObject(ObjectType::Instance, Type::of(k->name)), klass(k) {}
    
    std::string toString() const override {
        return "[Instance " + klass->name + "]";
    }
};

// Bound method (instance + method)
class BoundMethod : public RuntimeObject {
public:
    Instance* instance;
    Closure* method;
    
    BoundMethod(Instance* inst, Closure* m)
        : RuntimeObject(ObjectType::BoundMethod, Type::function()), instance(inst), method(m) {}
    
    std::string toString() const override {
        return "[BoundMethod " + method->function->name + "]";
    }
};

// Array object
class ArrayObj : public RuntimeObject {
public:
    std::vector<Value> elements;
    
    ArrayObj() : RuntimeObject(ObjectType::Array, Type::array()) {}
    ArrayObj(std::initializer_list<Value> elems) 
        : RuntimeObject(ObjectType::Array, Type::array()), elements(elems) {}
    
    Value& at(int index) { return elements[index]; }
    int length() const { return static_cast<int>(elements.size()); }
    
    std::string toString() const override {
        std::string result = "[";
        for (size_t i = 0; i < elements.size(); i++) {
            if (i > 0) result += ", ";
            result += elements[i].toString();
        }
        result += "]";
        return result;
    }
};

// String object
class StringObj : public RuntimeObject {
public:
    std::string value;
    
    StringObj() : RuntimeObject(ObjectType::String, Type::string()) {}
    StringObj(const std::string& v) : RuntimeObject(ObjectType::String, Type::string()), value(v) {}
    StringObj(const char* v) : RuntimeObject(ObjectType::String, Type::string()), value(v) {}
    
    int length() const { return static_cast<int>(value.length()); }
    
    std::string toString() const override {
        return "\"" + value + "\"";
    }
};

// Map/Object
class MapObj : public RuntimeObject {
public:
    std::unordered_map<std::string, Value> entries;
    
    MapObj() : RuntimeObject(ObjectType::Map, Type::object()) {}
    
    std::string toString() const override {
        std::string result = "{";
        bool first = true;
        for (const auto& entry : entries) {
            if (!first) result += ", ";
            result += entry.first + ": " + entry.second.toString();
            first = false;
        }
        result += "}";
        return result;
    }
};

} // namespace hs

#endif // HARVIS_VM_OBJECT_H
