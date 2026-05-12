#include "types.h"
#include "../vm/object.h"
#include <limits>

namespace hs {

// Destructor - critical for memory safety
Value::~Value() {
    switch (kind) {
        case Kind::String:
            if (payload.asString) {
                delete payload.asString;
            }
            break;
        case Kind::Object:
            // GC manages lifecycle - no manual release
            break;
        default:
            break;
    }
}

Value::Value(const std::string& s) 
    : kind(Kind::String), payload(new std::string(s)), type(Type::string()) {}

Value::Value(const char* s)
    : kind(Kind::String), payload(new std::string(s)), type(Type::string()) {}

 Value::Value(RuntimeObject* obj, Type t)
    : kind(Kind::Object), payload(obj), type(t) {}

Value::Value(Value&& other) noexcept 
    : kind(other.kind), type(std::move(other.type)) {
    switch (other.kind) {
        case Kind::Undefined:
        case Kind::Null:
            payload.asNumber = 0;
            break;
        case Kind::Boolean:
            payload.asBool = other.payload.asBool;
            break;
        case Kind::Number:
            payload.asNumber = other.payload.asNumber;
            break;
        case Kind::String:
            payload.asString = other.payload.asString;
            other.payload.asString = nullptr;
            break;
        case Kind::Object:
            payload.asObject = other.payload.asObject;
            other.payload.asObject = nullptr;
            break;
    }
}

Value& Value::operator=(Value&& other) noexcept {
    if (this == &other) return *this;
    
    // Clean up existing value
    if (kind == Kind::String && payload.asString) {
        delete payload.asString;
    }
    
    kind = other.kind;
    type = std::move(other.type);
    
    switch (other.kind) {
        case Kind::Undefined:
        case Kind::Null:
            payload.asNumber = 0;
            break;
        case Kind::Boolean:
            payload.asBool = other.payload.asBool;
            break;
        case Kind::Number:
            payload.asNumber = other.payload.asNumber;
            break;
        case Kind::String:
            payload.asString = other.payload.asString;
            other.payload.asString = nullptr;
            break;
        case Kind::Object:
            payload.asObject = other.payload.asObject;
            other.payload.asObject = nullptr;
            break;
    }
    
    return *this;
}

Value::Value(const Value& other)
    : kind(other.kind), type(other.type) {
    switch (other.kind) {
        case Kind::Undefined:
        case Kind::Null:
            payload.asNumber = 0;
            break;
        case Kind::Boolean:
            payload.asBool = other.payload.asBool;
            break;
        case Kind::Number:
            payload.asNumber = other.payload.asNumber;
            break;
        case Kind::String:
            payload.asString = other.payload.asString ? new std::string(*other.payload.asString) : nullptr;
            break;
        case Kind::Object:
            payload.asObject = other.payload.asObject;
            break;
    }
}

Value& Value::operator=(const Value& other) {
    if (this == &other) return *this;
    
    // Clean up existing value
    if (kind == Kind::String && payload.asString) {
        delete payload.asString;
    }
    // GC manages lifecycle - no manual release on object reassignment
    
    kind = other.kind;
    type = other.type;
    
    switch (other.kind) {
        case Kind::Undefined:
        case Kind::Null:
            payload.asNumber = 0;
            break;
        case Kind::Boolean:
            payload.asBool = other.payload.asBool;
            break;
        case Kind::Number:
            payload.asNumber = other.payload.asNumber;
            break;
        case Kind::String:
            payload.asString = other.payload.asString ? new std::string(*other.payload.asString) : nullptr;
            break;
        case Kind::Object:
            payload.asObject = other.payload.asObject;
            break;
    }
    
    return *this;
}

bool Value::toBool() const {
    switch (kind) {
        case Kind::Undefined: return false;
        case Kind::Null: return false;
        case Kind::Boolean: return payload.asBool;
        case Kind::Number: return payload.asNumber != 0.0;
        case Kind::String: return payload.asString ? !payload.asString->empty() : false;
        case Kind::Object: return true;
    }
    return false;
}

double Value::toNumber() const {
    switch (kind) {
        case Kind::Undefined: return 0.0;
        case Kind::Null: return 0.0;
        case Kind::Boolean: return payload.asBool ? 1.0 : 0.0;
        case Kind::Number: return payload.asNumber;
        case Kind::String: {
            if (payload.asString) {
                try {
                    return std::stod(*payload.asString);
                } catch (...) {
                    return std::numeric_limits<double>::quiet_NaN();
                }
            }
            return 0.0;
        }
        case Kind::Object: return std::numeric_limits<double>::quiet_NaN();
    }
    return 0.0;
}

std::string Value::toString() const {
    switch (kind) {
        case Kind::Undefined: return "undefined";
        case Kind::Null: return "null";
        case Kind::Boolean: return payload.asBool ? "true" : "false";
        case Kind::Number: return std::to_string(payload.asNumber);
        case Kind::String: return payload.asString ? *payload.asString : "";
        case Kind::Object: {
            if (payload.asObject) {
                return payload.asObject->toString();
            }
            return "[object " + std::string(type.name.value_or("Object")) + "]";
        }
    }
    return "";
}

bool Value::operator==(const Value& other) const {
    if (kind != other.kind) {
        // Type coercion for equality
        if (kind == Kind::Number && other.kind == Kind::String) {
            return toNumber() == other.toNumber();
        }
        if (kind == Kind::String && other.kind == Kind::Number) {
            return toNumber() == other.toNumber();
        }
        return false;
    }
    
    switch (kind) {
        case Kind::Undefined:
        case Kind::Null:
            return true;
        case Kind::Boolean:
            return payload.asBool == other.payload.asBool;
        case Kind::Number:
            return payload.asNumber == other.payload.asNumber;
        case Kind::String:
            if (!payload.asString || !other.payload.asString) {
                return payload.asString == other.payload.asString;
            }
            return *payload.asString == *other.payload.asString;
        case Kind::Object:
            return payload.asObject == other.payload.asObject;
    }
    return false;
}

// Scope implementation
Scope::Scope(std::weak_ptr<Scope> parent) : parentScope(parent) {}

void Scope::define(const std::string& name, Value value, Type type) {
    variables[name] = std::move(value);
    this->types[name] = type;
}

void Scope::assign(const std::string& name, Value value) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        it->second = std::move(value);
    }
}

std::optional<Value> Scope::get(const std::string& name) const {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    
    if (auto parent = parentScope.lock()) {
        return parent->get(name);
    }
    
    return std::nullopt;
}

std::optional<Type> Scope::getType(const std::string& name) const {
    auto it = types.find(name);
    if (it != types.end()) {
        return it->second;
    }
    
    if (auto parent = parentScope.lock()) {
        return parent->getType(name);
    }
    
    return std::nullopt;
}

bool Scope::has(const std::string& name) const {
    if (variables.find(name) != variables.end()) {
        return true;
    }
    
    if (auto parent = parentScope.lock()) {
        return parent->has(name);
    }
    
    return false;
}

// Environment implementation
Environment::Environment() {
    scopes.push_back(std::make_shared<Scope>());
}

Environment::Environment(std::shared_ptr<Scope> scope) {
    scopes.push_back(std::move(scope));
}

void Environment::beginScope() {
    auto parent = scopes.back();
    scopes.push_back(std::make_shared<Scope>(parent));
}

void Environment::endScope() {
    if (scopes.size() > 1) {
        scopes.pop_back();
    }
}

void Environment::define(const std::string& name, Value value, Type type) {
    scopes.back()->define(name, std::move(value), type);
}

void Environment::assign(const std::string& name, Value value) {
    scopes.back()->assign(name, std::move(value));
}

std::optional<Value> Environment::get(const std::string& name) const {
    return scopes.back()->get(name);
}

std::optional<Type> Environment::getType(const std::string& name) const {
    return scopes.back()->getType(name);
}

std::shared_ptr<Scope> Environment::currentScope() const {
    return scopes.back();
}

} // namespace hs
