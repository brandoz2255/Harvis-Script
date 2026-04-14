#ifndef HARVIS_CORE_TYPES_H
#define HARVIS_CORE_TYPES_H

#include <string>
#include <string_view>
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <vector>

namespace hs {

// Forward declarations
struct Value;
class RuntimeObject;
class Closure;
class Function;
class Instance;
class NativeFunction;
class BoundMethod;
class ArrayObj;
class StringObj;
class MapObj;

// Source location for error reporting
struct SourceLocation {
    std::string filename;
    int line = 0;
    int column = 0;

    bool operator==(const SourceLocation& other) const {
        return filename == other.filename && line == other.line && column == other.column;
    }

    std::string toString() const {
        std::string result = filename;
        if (line > 0) {
            result += ":" + std::to_string(line);
            if (column > 0) result += ":" + std::to_string(column);
        }
        return result;
    }
};

// Type system - supports gradual typing
enum class TypeTag {
    Undefined,
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object,
    Function,
    Native,
    Class,
    Module
};

// Type representation for gradual typing
struct Type {
    TypeTag tag;
    std::optional<std::string> name;  // For named types (classes, modules)
    std::vector<Type> params;         // For generics like Array<number>

    static Type undefined() { return {TypeTag::Undefined, std::nullopt, {}}; }
    static Type null() { return {TypeTag::Null, std::nullopt, {}}; }
    static Type boolean() { return {TypeTag::Boolean, std::nullopt, {}}; }
    static Type number() { return {TypeTag::Number, std::nullopt, {}}; }
    static Type string() { return {TypeTag::String, std::nullopt, {}}; }
    static Type array() { return {TypeTag::Array, std::nullopt, {}}; }
    static Type object() { return {TypeTag::Object, std::nullopt, {}}; }
    static Type function() { return {TypeTag::Function, std::nullopt, {}}; }
    static Type of(std::string_view name) { return {TypeTag::Object, std::string(name), {}}; }
};

// Runtime value - tagged union
struct Value {
    enum class Kind {
        Undefined,
        Null,
        Boolean,
        Number,
        String,
        Object  // Heap-allocated: Function, Closure, Instance, Array, Map, etc.
    };

    union Payload {
        bool asBool;
        double asNumber;
        std::string* asString;  // Owned string
        RuntimeObject* asObject;

        Payload() : asNumber(0) {}
        Payload(bool b) : asBool(b) {}
        Payload(double n) : asNumber(n) {}
        Payload(std::string* s) : asString(s) {}
        Payload(RuntimeObject* o) : asObject(o) {}
        ~Payload() {}
    };

    Kind kind;
    Payload payload;
    Type type;  // Static type info for gradual typing

    Value() : kind(Kind::Undefined), type(Type::undefined()) {}
    Value(std::nullptr_t) : kind(Kind::Null), type(Type::null()) {}
    Value(bool b) : kind(Kind::Boolean), payload(b), type(Type::boolean()) {}
    Value(double n) : kind(Kind::Number), payload(n), type(Type::number()) {}
    Value(const std::string& s);
    Value(const char* s);
    Value(RuntimeObject* obj, Type t = Type::object());
    ~Value();
    
    // Move constructors for owned types
    Value(Value&& other) noexcept;
    Value& operator=(Value&& other) noexcept;
    
    // Copy constructor
    Value(const Value& other);
    Value& operator=(const Value& other);

    bool isUndefined() const { return kind == Kind::Undefined; }
    bool isNull() const { return kind == Kind::Null; }
    bool isBoolean() const { return kind == Kind::Boolean; }
    bool isNumber() const { return kind == Kind::Number; }
    bool isString() const { return kind == Kind::String; }
    bool isObject() const { return kind == Kind::Object; }

    bool toBool() const;
    double toNumber() const;
    std::string toString() const;
    RuntimeObject* asObject() const { return kind == Kind::Object ? payload.asObject : nullptr; }

    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const { return !(*this == other); }
};

// Scope for variable resolution
class Scope {
public:
    Scope(std::weak_ptr<Scope> parent = std::weak_ptr<Scope>());

    void define(const std::string& name, Value value, Type type = Type::undefined());
    void assign(const std::string& name, Value value);
    std::optional<Value> get(const std::string& name) const;
    std::optional<Type> getType(const std::string& name) const;
    bool has(const std::string& name) const;

private:
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, Type> types;
    std::weak_ptr<Scope> parentScope;
};

// Environment (chain of scopes)
class Environment {
public:
    Environment();
    explicit Environment(std::shared_ptr<Scope> scope);

    void beginScope();
    void endScope();

    void define(const std::string& name, Value value, Type type = Type::undefined());
    void assign(const std::string& name, Value value);
    std::optional<Value> get(const std::string& name) const;
    std::optional<Type> getType(const std::string& name) const;

    std::shared_ptr<Scope> currentScope() const;

private:
    std::vector<std::shared_ptr<Scope>> scopes;
};

} // namespace hs

#endif // HARVIS_CORE_TYPES_H
