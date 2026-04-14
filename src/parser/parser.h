#ifndef HARVIS_PARSER_PARSER_H
#define HARVIS_PARSER_PARSER_H

#include "../lexer/lexer.h"
#include "../ast/ast.h"
#include <memory>
#include <stdexcept>

namespace hs {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    
    Program parse();
    std::vector<Stmt::Ptr> parseStatements();
    
    // Error handling
    const std::vector<ParserError>& getErrors() const { return errors; }
    bool hasErrors() const { return !errors.empty(); }
    void clearErrors() { errors.clear(); }
    
private:
    std::vector<Token> tokens;
    std::vector<ParserError> errors;
    int current = 0;
    
    // Check if we've reached EOF
    bool isAtEnd() const { return peek().type == TokenType::EOF_TOKEN; }
    
    // Token access
    const Token& peek() const { return tokens[current]; }
    const Token& previous() const { return tokens[current - 1]; }
    const Token& advance();
    
    // Synchronization
    void synchronize();
    
    // Matching
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(TokenType t1, TokenType t2);
    bool match(TokenType t1, TokenType t2, TokenType t3);
    bool match(TokenType t1, TokenType t2, TokenType t3, TokenType t4);
    bool checkAny(const std::vector<TokenType>& types) const;
    bool matchAny(const std::vector<TokenType>& types);
    bool previousIs(TokenType type) const;
    
    // Error handling
    void error(const Token& token, const std::string& message);
    void error(const SourceLocation& loc, const std::string& message);
    void error(ParserError::Kind kind, const std::string& detail = "");
    
// Statement parsing
    Stmt::Ptr parseStatement();
    Stmt::Ptr parseDeclaration();
    Stmt::Ptr parseBlockStatement();
    Stmt::Ptr parseExpressionStatement();
    std::vector<Stmt::Ptr> parseBlock();
    
    // Expression parsing (precedence climbing)
    Expr::Ptr parseExpression();
    Expr::Ptr parseAssignment();
    Expr::Ptr parseLogicalOr();
    Expr::Ptr parseLogicalAnd();
    Expr::Ptr parseBitwiseOr();
    Expr::Ptr parseBitwiseXor();
    Expr::Ptr parseBitwiseAnd();
    Expr::Ptr parseEquality();
    Expr::Ptr parseComparison();
    Expr::Ptr parseShift();
    Expr::Ptr parseAdditive();
    Expr::Ptr parseMultiplicative();
    Expr::Ptr parseUnary();
    Expr::Ptr parseCall();
    Expr::Ptr parsePrimary();
    
    // Helper methods
    bool consume(TokenType type, const std::string& message);
    std::vector<Expr::Ptr> parseArgumentList();
    std::vector<Expr::Ptr> parseCommaSeparatedExpressions();
    std::vector<std::pair<std::string, Type>> parseParameterList();
    
    // Statement parsing
    Stmt::Ptr parseClass();
    Stmt::Ptr parseFunction();
    Stmt::Ptr parseVariable(bool isLet);
    Stmt::Ptr parseConst();
    Stmt::Ptr parseIf();
    Stmt::Ptr parseWhile();
    Stmt::Ptr parseFor();
    Stmt::Ptr parseDoWhile();
    Stmt::Ptr parseReturn();
    Stmt::Ptr parseBreak();
    Stmt::Ptr parseContinue();
    Stmt::Ptr parseImport();
    Stmt::Ptr parseExport();
    Stmt::Ptr parseTry();
    Stmt::Ptr parseThrow();
    Stmt::Ptr parseSwitch();
    
    // Type parsing (for gradual typing)
    Type parseType();
    std::string parseTypeAnnotation();
};

// Convenience function
Program parse(const std::string& source, const std::string& filename = "<script>");

} // namespace hs

#endif // HARVIS_PARSER_PARSER_H
