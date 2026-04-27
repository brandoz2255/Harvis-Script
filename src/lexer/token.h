#ifndef HARVIS_LEXER_TOKEN_H
#define HARVIS_LEXER_TOKEN_H

#include "../core/types.h"
#include <string>
#include <variant>
#include <iostream>

namespace hs {

// Token types for Harvis Script
enum class TokenType {
    // Literals
    IDENTIFIER,
    STRING,
    NUMBER,
    
    // Keywords (gradual typing)
    TYPE_KEYWORD,      // type
    INTERFACE_KEYWORD, // interface
    EXTENDS_KEYWORD,   // extends
    IMPLEMENTS_KEYWORD, // implements
    ENUM_KEYWORD,      // enum
    CONST_KEYWORD,     // const
    LET_KEYWORD,       // let
    VAR_KEYWORD,       // var
    
    // Control flow
    IF_KEYWORD,
    ELSE_KEYWORD,
    WHILE_KEYWORD,
    FOR_KEYWORD,
    DO_KEYWORD,
    BREAK_KEYWORD,
    CONTINUE_KEYWORD,
    RETURN_KEYWORD,
    SWITCH_KEYWORD,
    CASE_KEYWORD,
    DEFAULT_KEYWORD,
   RANGE_KEYWORD,
    IN_KEYWORD,
    TYPE_SWITCH_KEYWORD,
    STRUCT_KEYWORD,
    DEFER_KEYWORD,
    PANIC_KEYWORD,
    RECOVER_KEYWORD,
    GO_KEYWORD,
    CHANNEL_KEYWORD,
    SELECT_KEYWORD,
    MUTEX_KEYWORD,
    WAITGROUP_KEYWORD,
    
    // Functions & classes
    FUNCTION_KEYWORD,  // function
    CLASS_KEYWORD,     // class
    CONSTRUCTOR_KEYWORD, // constructor
    NEW_KEYWORD,       // new
    SUPER_KEYWORD,     // super
    THIS_KEYWORD,      // this
    EXPORT_KEYWORD,    // export
    IMPORT_KEYWORD,    // import
    FROM_KEYWORD,      // from
    
    // Modules & types
    MODULE_KEYWORD,    // module
    NAMESPACE_KEYWORD, // namespace
    AS_KEYWORD,        // as
    PUBLIC_KEYWORD,    // public
    PRIVATE_KEYWORD,   // private
    PROTECTED_KEYWORD, // protected
    STATIC_KEYWORD,    // static
    ABSTRACT_KEYWORD,  // abstract
    FINAL_KEYWORD,     // final
    
    // Type annotations
    ANY_KEYWORD,       // any
    UNKNOWN_KEYWORD,   // unknown
    NEVER_KEYWORD,     // never
    VOID_KEYWORD,      // void
    NULL_KEYWORD,      // null
    TRUE_KEYWORD,      // true
    FALSE_KEYWORD,     // false
    UNDEFINED_KEYWORD, // undefined
    
    // Exception handling
    TRY_KEYWORD,
    CATCH_KEYWORD,
    THROW_KEYWORD,
    FINALLY_KEYWORD,
    
    // Operators
    EQUAL,           // =
    EQUAL_EQUAL,     // ==
    BANG_EQUAL,      // !=
    LESS,            // <
    LESS_EQUAL,      // <=
    GREATER,         // >
    GREATER_EQUAL,   // >=
    BANG,            // !
    BANG_EQUAL_OLD,  // != (alternative)
    
    PLUS,            // +
    MINUS,           // -
    STAR,            // *
    SLASH,           // /
    PERCENT,         // %
    EXCLAMATION,     // !
    
    PLUS_PLUS,       // ++
    MINUS_MINUS,     // --
    PLUS_EQUAL,      // +=
    MINUS_EQUAL,     // -=
    STAR_EQUAL,      // *=
    SLASH_EQUAL,     // /=
    PERCENT_EQUAL,   // %=
    
    AND,             // &&
    OR,              // ||
    NULLISH_COALESCE, // ??
    QUESTION,        // ?
    COLON,           // :
    SEMICOLON,       // ;
    COMMA,           // ,
    
    // Access & delimiters
    DOT,             // .
    DOT_QUESTION,    // ?. (optional chaining)
    DOT_DOT,         // .. (spread/rest)
    DOT_DOT_DOT,     // ... (rest/spread)
    ARROW,           // =>
    AT,              // @ (decorators)
    TILDE,           // ~ (bitwise not)
    AMPERSAND,       // & (bitwise and)
    PIPE,            // | (bitwise or)
    CARET,           // ^ (bitwise xor)
    LESS_LESS,       // << (left shift)
    GREATER_GREATER, // >> (right shift)
    RIGHT_ANGLE,     // > (for type params)
    TILDE_EQUALS,    // ~= (approximate)
    
    // Delimiters
    LEFT_PAREN,      // (
    RIGHT_PAREN,     // )
    LEFT_BRACKET,    // [
    RIGHT_BRACKET,   // ]
    LEFT_BRACE,      // {
    RIGHT_BRACE,     // }
    
    // Special
    EOF_TOKEN,
    ERROR
};

// Token literal values
struct TokenLiteral {
    std::string stringValue;
    double numberValue;
    TokenType type;
    
    TokenLiteral(const std::string& s) : stringValue(s), numberValue(0), type(TokenType::STRING) {}
    TokenLiteral(double n) : stringValue(), numberValue(n), type(TokenType::NUMBER) {}
    TokenLiteral(TokenType t) : stringValue(), numberValue(0), type(t) {}
    
    std::string toString() const {
        if (type == TokenType::NUMBER) return std::to_string(numberValue);
        return stringValue;
    }
    
    bool isNumber() const { return type == TokenType::NUMBER; }
    bool isString() const { return type == TokenType::STRING; }
};

// Token representation
struct Token {
    TokenType type;
    std::string lexeme;
    TokenLiteral literal;
    SourceLocation location;
    
    Token(TokenType t, const std::string& lex, const TokenLiteral& lit, const SourceLocation& loc)
        : type(t), lexeme(lex), literal(lit), location(loc) {}
    
    Token(TokenType t, const std::string& lex, SourceLocation loc)
        : type(t), lexeme(lex), literal(t), location(loc) {}
    
    Token(TokenType t, SourceLocation loc)
        : type(t), lexeme(tokenTypeToString(t)), literal(t), location(loc) {}
    
    bool is(TokenType t) const { return type == t; }
    bool matches(TokenType t1, TokenType t2) const { return type == t1 || type == t2; }
    
    static std::string tokenTypeToString(TokenType t);
};

// Token stream iterator
class TokenIterator {
public:
    TokenIterator(const std::vector<Token>& tokens) : tokens(tokens), index(0) {}
    
    bool hasNext() const { return index < tokens.size(); }
    const Token& current() const { return tokens[index]; }
    const Token& next();
    void advance() { ++index; }
    
private:
    const std::vector<Token>& tokens;
    size_t index;
};

// Keyword lookup
class KeywordTable {
public:
    static TokenType get(const std::string& identifier);
    static bool isKeyword(const std::string& identifier);
    static bool isTypeKeyword(TokenType type);
};

// Pretty print tokens
std::ostream& operator<<(std::ostream& os, const Token& token);

} // namespace hs

#endif // HARVIS_LEXER_TOKEN_H
