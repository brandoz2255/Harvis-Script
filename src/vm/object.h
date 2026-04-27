#ifndef HARVIS_VM_OBJECT_H
#define HARVIS_VM_OBJECT_H

#include "../core/types.h"
#include "../compiler/chunk.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace hs {

// Forward declaration
class Upvalue;

// Object types
enum class ObjectType {
    None,
    Function,
    Closure,
    Class,
    Instance,
    Native,
    BoundMethod,
    Array,
    String,
    Map,
    Channel,
    Mutex,
    WaitGroup
};

// Base object class with reference counting
class RuntimeObject {
public:
    ObjectType type;
    Type declaredType;
    int refCount;
    
    RuntimeObject(ObjectType t, Type dt = Type::object()) : type(t), declaredType(dt), refCount(1) {}
    
    void retain() { refCount++; }
    void release() { 
        if (--refCount <= 0) delete this; 
    }
    
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

// Class object (stores methods and acts as factory for instances)
class ClassObj : public RuntimeObject {
public:
    std::string name;
    std::unordered_map<std::string, Closure*> methods;
    Function* superClass;
    
    ClassObj(const std::string& n, Function* super = nullptr)
        : RuntimeObject(ObjectType::Class, Type::object()), name(n), superClass(super) {}
    
    std::string toString() const override {
        return "[Class " + name + "]";
    }
};

// Class instance
class Instance : public RuntimeObject {
public:
    ClassObj* klass;
    std::unordered_map<std::string, Value> fields;
    
    Instance(ClassObj* k) : RuntimeObject(ObjectType::Instance, Type::of(k->name)), klass(k) {}
    
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

// Channel object (Go-style buffered channel)
class ChannelObj : public RuntimeObject {
public:
    int capacity;
    std::queue<Value> queue;
    std::mutex mtx;
    std::condition_variable notFull;
    std::condition_variable notEmpty;
    bool closed;
    
    ChannelObj(int cap = 0) : RuntimeObject(ObjectType::Channel, Type::channel()), capacity(cap), closed(false) {}
    
    void send(Value value) {
        std::unique_lock<std::mutex> lock(mtx);
        if (static_cast<int>(queue.size()) >= capacity) {
            notFull.wait(lock, [this]() { return static_cast<int>(queue.size()) < capacity || closed; });
        }
        if (closed) return;
        queue.push(std::move(value));
        notEmpty.notify_one();
    }
    
    Value receive() {
        std::unique_lock<std::mutex> lock(mtx);
        if (queue.empty() && !closed) {
            notEmpty.wait(lock, [this]() { return !queue.empty() || closed; });
        }
        if (queue.empty() && closed) {
            return Value();
        }
        Value value = std::move(queue.front());
        queue.pop();
        notFull.notify_one();
        return value;
    }
    
    void close() {
        std::unique_lock<std::mutex> lock(mtx);
        closed = true;
        notEmpty.notify_all();
        notFull.notify_all();
    }
    
    bool isClosed() {
        std::unique_lock<std::mutex> lock(mtx);
        return closed;
    }
    
    std::string toString() const override {
        return "[Channel cap=" + std::to_string(capacity) + " len=" + std::to_string(queue.size()) + "]";
    }
};

// Mutex object (Go-style sync.Mutex)
class MutexObj : public RuntimeObject {
public:
    std::mutex mtx;
    
    MutexObj() : RuntimeObject(ObjectType::Mutex, Type::mutex()) {}
    
    void lock() { mtx.lock(); }
    void unlock() { mtx.unlock(); }
    
    std::string toString() const override {
        return "[Mutex]";
    }
};

// WaitGroup object (Go-style sync.WaitGroup)
class WaitGroupObj : public RuntimeObject {
public:
    int counter;
    std::mutex mtx;
    std::condition_variable cv;
    
    WaitGroupObj() : RuntimeObject(ObjectType::WaitGroup, Type::waitgroup()), counter(0) {}
    
    void add(int delta) {
        std::lock_guard<std::mutex> lock(mtx);
        counter += delta;
    }
    
    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return counter <= 0; });
    }
    
    void done() {
        std::lock_guard<std::mutex> lock(mtx);
        counter--;
        if (counter <= 0) {
            cv.notify_all();
        }
    }
    
    int count() {
        std::lock_guard<std::mutex> lock(mtx);
        return counter;
    }
    
    std::string toString() const override {
        return "[WaitGroup counter=" + std::to_string(counter) + "]";
    }
};

} // namespace hs

#endif // HARVIS_VM_OBJECT_H
