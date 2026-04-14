#ifndef HARVIS_CORE_ERROR_H
#define HARVIS_CORE_ERROR_H

#include "types.h"
#include <stdexcept>
#include <string>
#include <format>

namespace hs {

// Compile-time errors (lexer/parser/compiler)
struct CompileError : std::runtime_error {
    SourceLocation location;
    std::string message;
    std::string context;

    CompileError() : std::runtime_error("") {}
    CompileError(const SourceLocation& loc, const std::string& msg, const std::string& ctx = "")
        : std::runtime_error(formatMessage(loc, msg, ctx)), location(loc), message(msg), context(ctx) {}

    static std::string formatMessage(const SourceLocation& loc, const std::string& msg, const std::string& ctx) {
        std::string result = loc.toString() + ": error: " + msg;
        if (!ctx.empty()) result += "\n  " + ctx;
        return result;
    }
};

// Runtime errors (VM)
struct RuntimeError : std::runtime_error {
    RuntimeError(const std::string& msg) : std::runtime_error(msg) {}
};

// Type errors for gradual type checking
struct TypeError : RuntimeError {
    Type expected;
    Type actual;

    TypeError(const Type& exp, const Type& act, const std::string& msg)
        : RuntimeError(msg), expected(exp), actual(act) {}
};

// Lexer errors
struct LexerError : CompileError {
    enum class Kind {
        UnexpectedCharacter,
        UnterminatedString,
        InvalidNumber,
        InvalidUnicodeEscape,
        InvalidEscapeSequence,
        InvalidIdentifier
    };

    LexerError(const SourceLocation& loc, Kind kind, const std::string& detail = "")
        : CompileError(loc, messageForKind(kind), detail) {}

    static std::string messageForKind(Kind kind) {
        switch (kind) {
            case Kind::UnexpectedCharacter: return "unexpected character";
            case Kind::UnterminatedString: return "unterminated string literal";
            case Kind::InvalidNumber: return "invalid number literal";
            case Kind::InvalidUnicodeEscape: return "invalid unicode escape";
            case Kind::InvalidEscapeSequence: return "invalid escape sequence";
            case Kind::InvalidIdentifier: return "invalid identifier";
            default: return "unknown lexer error";
        }
    }
};

// Parser errors
struct ParserError : CompileError {
    enum class Kind {
        UnexpectedToken,
        ExpectedToken,
        UnexpectedEOF,
        SyntaxError
    };

    ParserError(const SourceLocation& loc, Kind kind, const std::string& detail = "")
        : CompileError(loc, messageForKind(kind), detail) {}

    static std::string messageForKind(Kind kind) {
        switch (kind) {
            case Kind::UnexpectedToken: return "unexpected token";
            case Kind::ExpectedToken: return "expected token";
            case Kind::UnexpectedEOF: return "unexpected end of file";
            case Kind::SyntaxError: return "syntax error";
            default: return "unknown parser error";
        }
    }
};

// Compiler errors
struct CompilerError : CompileError {
    enum class Kind {
        UndefinedVariable,
        TypeMismatch,
        MissingReturn,
        InvalidOperation
    };

    CompilerError(const SourceLocation& loc, Kind kind, const std::string& detail = "")
        : CompileError(loc, messageForKind(kind), detail) {}

    static std::string messageForKind(Kind kind) {
        switch (kind) {
            case Kind::UndefinedVariable: return "undefined variable";
            case Kind::TypeMismatch: return "type mismatch";
            case Kind::MissingReturn: return "missing return statement";
            case Kind::InvalidOperation: return "invalid operation";
        }
    }
};

// Result type for error handling without exceptions
template<typename T>
class Result {
public:
    Result(T&& value) : ok(true), value(std::move(value)), error(SourceLocation{}, "") {}
    Result(const T& value) : ok(true), value(value), error(SourceLocation{}, "") {}
    Result(const CompileError& err) : ok(false), error(err) {}

    bool isSuccess() const { return ok; }
    bool isError() const { return !ok; }

    T& operator*() { return value; }
    const T& operator*() const { return value; }
    T* operator->() { return &value; }
    const T* operator->() const { return &value; }

    T& valueOr(T&& defaultVal) { return ok ? value : (value = std::move(defaultVal)); }
    const T& valueOr(const T& defaultVal) const { return ok ? value : defaultVal; }

    const CompileError& getError() const { return error; }

private:
    bool ok;
    T value;
    CompileError error;
};

// Panic macro for unrecoverable errors
#define HS_PANIC(msg) throw std::runtime_error(std::string("Panic: ") + msg)

} // namespace hs

#endif // HARVIS_CORE_ERROR_H
