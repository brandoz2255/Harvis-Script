#ifndef HARVIS_LEXER_LEXER_H
#define HARVIS_LEXER_LEXER_H

#include "token.h"
#include "../core/scanner.h"
#include "../core/error.h"
#include <vector>
#include <memory>
#include <functional>

namespace hs {

class Lexer {
public:
    explicit Lexer(const std::string& source, const std::string& filename = "<script>");
    
    std::vector<Token> tokenize();
    Result<std::vector<Token>> tokenizeWithErrorHandling();
    
    // Get the last error that occurred
    const std::optional<LexerError>& getLastError() const { return lastError; }
    const std::vector<LexerError>& getErrors() const { return errors; }
    
    // Check if there are errors
    bool hasErrors() const { return !errors.empty(); }
    
    // Clear errors
    void clearErrors() { errors.clear(); lastError.reset(); }

private:
    std::string source;
    std::string filename;
    CharBuffer buffer;
    std::vector<Token> tokens;
    std::vector<LexerError> errors;
    std::optional<LexerError> lastError;
    
    // Token scanning methods
    void scanToken();
    void addToken(TokenType type, const std::string& lexeme);
    void addToken(TokenType type);
    void addNumberToken(double value);
    
    // Literal scanners
    void scanString(char quote);
    void scanNumber();
    void scanIdentifier();
    
    // Comment handling
    void skipComment();
    void skipLineComment();
    void skipBlockComment();
    
    // Helper methods
    bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    bool isAlphaNum(char c) { return isAlpha(c) || (c >= '0' && c <= '9'); }
    bool isDigit(char c) { return c >= '0' && c <= '9'; }
    
    // Error handling
    void reportError(LexerError::Kind kind, const std::string& detail = "");
    
    // Multi-character operators
    TokenType scanOperator();
};

// Convenience function for one-off lexing
std::vector<Token> tokenize(const std::string& source, const std::string& filename = "<script>");

} // namespace hs

#endif // HARVIS_LEXER_LEXER_H
