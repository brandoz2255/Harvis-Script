#include "parser.h"
#include <algorithm>

namespace hs {

Parser::Parser(const std::vector<Token>& toks) : tokens(toks) {}

Program Parser::parse() {
    auto statements = parseStatements();
    if (isAtEnd()) {
        return Program("", statements, "");
    }
    return Program("", statements, "");
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        ++current;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(TokenType t1, TokenType t2) {
    if (check(t1) || check(t2)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(TokenType t1, TokenType t2, TokenType t3) {
    if (check(t1) || check(t2) || check(t3)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(TokenType t1, TokenType t2, TokenType t3, TokenType t4) {
    if (check(t1) || check(t2) || check(t3) || check(t4)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::checkAny(const std::vector<TokenType>& types) const {
    for (auto t : types) if (check(t)) return true;
    return false;
}

bool Parser::matchAny(const std::vector<TokenType>& types) {
    if (checkAny(types)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::previousIs(TokenType type) const {
    return previous().type == type;
}

void Parser::error(const Token& token, const std::string& message) {
    if (token.type == TokenType::EOF_TOKEN) {
        errors.push_back(ParserError(token.location, ParserError::Kind::UnexpectedEOF, message));
    } else {
        errors.push_back(ParserError(token.location, ParserError::Kind::SyntaxError, message));
    }
}

void Parser::error(const SourceLocation& loc, const std::string& message) {
    errors.push_back(ParserError(loc, ParserError::Kind::SyntaxError, message));
}

void Parser::error(ParserError::Kind kind, const std::string& detail) {
    errors.push_back(ParserError(peek().location, kind, detail));
}

bool Parser::consume(TokenType type, const std::string& message) {
    if (match(type)) return true;
    error(peek(), message);
    return false;
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        
        switch (peek().type) {
            case TokenType::CLASS_KEYWORD:
            case TokenType::FUNCTION_KEYWORD:
            case TokenType::VAR_KEYWORD:
            case TokenType::LET_KEYWORD:
            case TokenType::CONST_KEYWORD:
            case TokenType::IF_KEYWORD:
            case TokenType::FOR_KEYWORD:
            case TokenType::WHILE_KEYWORD:
            case TokenType::RETURN_KEYWORD:
            case TokenType::IMPORT_KEYWORD:
            case TokenType::EXPORT_KEYWORD:
                return;
            default:
                break;
        }
        advance();
    }
}

std::vector<Stmt::Ptr> Parser::parseStatements() {
    std::vector<Stmt::Ptr> statements;
    while (!isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) statements.push_back(stmt);
    }
    return statements;
}

Stmt::Ptr Parser::parseStatement() {
    try {
        if (match(TokenType::CLASS_KEYWORD)) return parseClass();
        if (match(TokenType::FUNCTION_KEYWORD)) return parseFunction();
        if (match(TokenType::VAR_KEYWORD)) return parseVariable(false);
        if (match(TokenType::LET_KEYWORD)) return parseVariable(true);
        if (match(TokenType::CONST_KEYWORD)) return parseConst();
        if (match(TokenType::IF_KEYWORD)) return parseIf();
        if (match(TokenType::WHILE_KEYWORD)) return parseWhile();
        if (match(TokenType::FOR_KEYWORD)) return parseFor();
        if (match(TokenType::DO_KEYWORD)) return parseDoWhile();
        if (match(TokenType::RETURN_KEYWORD)) return parseReturn();
        if (match(TokenType::BREAK_KEYWORD)) return parseBreak();
        if (match(TokenType::CONTINUE_KEYWORD)) return parseContinue();
        if (match(TokenType::IMPORT_KEYWORD)) return parseImport();
        if (match(TokenType::EXPORT_KEYWORD)) return parseExport();
        if (match(TokenType::TRY_KEYWORD)) return parseTry();
        if (match(TokenType::THROW_KEYWORD)) return parseThrow();
        if (match(TokenType::SWITCH_KEYWORD)) return parseSwitch();
        if (match(TokenType::LEFT_BRACE)) return parseBlockStatement();
        
        return parseExpressionStatement();
    } catch (const std::exception& e) {
        synchronize();
        return nullptr;
    }
}

Stmt::Ptr Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    if (match(TokenType::SEMICOLON)) {
        return std::make_shared<ExpressionStmt>(previous().location, expr);
    }
    return nullptr;
}

Stmt::Ptr Parser::parseBlockStatement() {
    auto start = previous().location;
    std::vector<Stmt::Ptr> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) statements.push_back(stmt);
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_shared<BlockStmt>(start, statements);
}

Stmt::Ptr Parser::parseClass() {
    auto name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        error(name, "Expect class name.");
        return nullptr;
    }
    
    // Skip to the end of the class for now
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        advance();
    }
    advance(); // consume }
    return nullptr;
}

Stmt::Ptr Parser::parseFunction() {
    std::string name = "";
    if (match(TokenType::IDENTIFIER)) {
        name = advance().lexeme;
    } else {
        // Anonymous function
    }
    
    SourceLocation loc = previous().location;
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    auto params = parseParameterList();
    
    std::string returnType;
    if (match(TokenType::COLON)) {
        returnType = parseTypeAnnotation();
    }
    
    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    std::vector<Stmt::Ptr> body;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) body.push_back(stmt);
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after function body.");
    
    return std::make_shared<FunctionStmt>(loc, name, params, returnType, body);
}

std::vector<std::pair<std::string, Type>> Parser::parseParameterList() {
    std::vector<std::pair<std::string, Type>> params;
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            auto param = advance();
            if (param.type != TokenType::IDENTIFIER) {
                error(param, "Expected parameter name.");
                continue;
            }
            params.emplace_back(param.lexeme, Type::undefined());
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    return params;
}

Stmt::Ptr Parser::parseVariable(bool isLet) {
    auto name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        error(name, "Expected variable name.");
        return nullptr;
    }
    
    SourceLocation loc = name.location;
    Type type;
    if (match(TokenType::COLON)) {
        type = parseType();
    }
    
    Expr::Ptr initializer;
    if (match(TokenType::EQUAL)) {
        initializer = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_shared<VarDeclStmt>(loc, name.lexeme, type, initializer, false, isLet);
}

Stmt::Ptr Parser::parseConst() {
    auto name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        error(name, "Expected constant name.");
        return nullptr;
    }
    
    SourceLocation loc = name.location;
    Expr::Ptr initializer;
    if (match(TokenType::EQUAL)) {
        initializer = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after constant declaration.");
    return std::make_shared<VarDeclStmt>(loc, name.lexeme, Type::undefined(), initializer, true, true);
}

Stmt::Ptr Parser::parseIf() {
    SourceLocation loc = previous().location;
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
    
    auto thenBranch = parseStatement();
    Stmt::Ptr elseBranch;
    
    if (match(TokenType::ELSE_KEYWORD)) {
        elseBranch = parseStatement();
    }
    
    return std::make_shared<IfStmt>(loc, condition, thenBranch, elseBranch);
}

Stmt::Ptr Parser::parseWhile() {
    SourceLocation loc = previous().location;
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");
    
    auto body = parseStatement();
    return std::make_shared<WhileStmt>(loc, condition, body);
}

Stmt::Ptr Parser::parseFor() {
    SourceLocation loc = previous().location;
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
    
    std::variant<Expr::Ptr, std::shared_ptr<VarDeclStmt>> init;
    if (match(TokenType::SEMICOLON)) {
        init = Expr::Ptr(nullptr);
    } else if (match(TokenType::VAR_KEYWORD)) {
        init = std::dynamic_pointer_cast<VarDeclStmt>(parseVariable(false));
    } else if (match(TokenType::LET_KEYWORD)) {
        init = std::dynamic_pointer_cast<VarDeclStmt>(parseVariable(true));
    } else {
        init = parseExpression();
    }
    
    Expr::Ptr condition;
    if (!check(TokenType::SEMICOLON)) {
        condition = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
    
    Expr::Ptr increment;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = parseExpression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");
    
    auto body = parseStatement();
    return std::make_shared<ForStmt>(loc, init, condition, increment, body);
}

Stmt::Ptr Parser::parseDoWhile() {
    SourceLocation loc = previous().location;
    auto body = parseStatement();
    
    consume(TokenType::WHILE_KEYWORD, "Expect 'while' after do statement.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after do-while condition.");
    consume(TokenType::SEMICOLON, "Expect ';' after do-while statement.");
    
    return std::make_shared<DoWhileStmt>(loc, body, condition);
}

Stmt::Ptr Parser::parseReturn() {
    SourceLocation loc = previous().location;
    
    Expr::Ptr value;
    if (!check(TokenType::SEMICOLON) && !isAtEnd()) {
        value = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_shared<ReturnStmt>(loc, value);
}

Stmt::Ptr Parser::parseBreak() {
    SourceLocation loc = previous().location;
    consume(TokenType::SEMICOLON, "Expect ';' after break.");
    return std::make_shared<BreakStmt>(loc);
}

Stmt::Ptr Parser::parseContinue() {
    SourceLocation loc = previous().location;
    consume(TokenType::SEMICOLON, "Expect ';' after continue.");
    return std::make_shared<ContinueStmt>(loc);
}

Stmt::Ptr Parser::parseImport() {
    SourceLocation loc = previous().location;
    
    // TODO: Parse import statement
    // For now, skip to the end
    while (!check(TokenType::SEMICOLON) && !isAtEnd()) {
        advance();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after import statement.");
    
    return std::make_shared<ImportStmt>(loc, "", std::vector<std::pair<std::string, std::string>>{});
}

Stmt::Ptr Parser::parseExport() {
    SourceLocation loc = previous().location;
    
    // TODO: Parse export statement
    while (!check(TokenType::SEMICOLON) && !isAtEnd()) {
        advance();
    }
    if (match(TokenType::SEMICOLON)) {}
    
    return std::make_shared<ExportStmt>(loc, false, Expr::Ptr(nullptr));
}

Stmt::Ptr Parser::parseTry() {
    SourceLocation loc = previous().location;
    
    consume(TokenType::LEFT_BRACE, "Expect '{' after 'try'.");
    std::vector<Stmt::Ptr> tryBody;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) tryBody.push_back(stmt);
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after try block.");
    auto tryBlock = std::make_shared<BlockStmt>(loc, tryBody);
    
    std::pair<std::string, Stmt::Ptr> catchClause("", nullptr);
    if (match(TokenType::CATCH_KEYWORD)) {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'catch'.");
        auto paramName = advance();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after catch parameter.");
        consume(TokenType::LEFT_BRACE, "Expect '{' after catch clause.");
        std::vector<Stmt::Ptr> catchBody;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            auto stmt = parseStatement();
            if (stmt) catchBody.push_back(stmt);
        }
        consume(TokenType::RIGHT_BRACE, "Expect '}' after catch block.");
        catchClause = {paramName.lexeme, std::make_shared<BlockStmt>(paramName.location, catchBody)};
    }
    
    Stmt::Ptr finallyBlock;
    if (match(TokenType::FINALLY_KEYWORD)) {
        consume(TokenType::LEFT_BRACE, "Expect '{' after 'finally'.");
        std::vector<Stmt::Ptr> finallyBody;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            auto stmt = parseStatement();
            if (stmt) finallyBody.push_back(stmt);
        }
        consume(TokenType::RIGHT_BRACE, "Expect '}' after finally block.");
        finallyBlock = std::make_shared<BlockStmt>(loc, finallyBody);
    }
    
    return std::make_shared<TryStmt>(loc, tryBlock, catchClause, finallyBlock);
}

Stmt::Ptr Parser::parseThrow() {
    SourceLocation loc = previous().location;
    auto value = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after thrown value.");
    return std::make_shared<ThrowStmt>(loc, value);
}

Stmt::Ptr Parser::parseSwitch() {
    SourceLocation loc = previous().location;
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'switch'.");
    auto expression = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after switch expression.");
    
    consume(TokenType::LEFT_BRACE, "Expect '{' before switch cases.");
    
    std::vector<SwitchStmt::Case> cases;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        SwitchStmt::Case cs;
        if (match(TokenType::CASE_KEYWORD)) {
            cs.condition = parseExpression();
        } else if (match(TokenType::DEFAULT_KEYWORD)) {
            cs.condition = nullptr;
        } else {
            break;
        }
        
        consume(TokenType::COLON, "Expect ':' after switch case.");
        
        while (!match(TokenType::CASE_KEYWORD) && !match(TokenType::DEFAULT_KEYWORD) && 
               !check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            auto stmt = parseStatement();
            if (stmt) cs.statements.push_back(stmt);
        }
        
        cases.push_back(cs);
    }
    
    consume(TokenType::RIGHT_BRACE, "Expect '}' after switch statement.");
    return std::make_shared<SwitchStmt>(loc, expression, cases);
}

// ============ EXPRESSION PARSING ============

Expr::Ptr Parser::parseExpression() {
    return parseAssignment();
}

Expr::Ptr Parser::parseAssignment() {
    auto expr = parseLogicalOr();
    
    if (match(TokenType::EQUAL)) {
        auto equals = advance();
        auto value = parseAssignment();
        
        // Check if we can assign to this
        if (auto ident = std::dynamic_pointer_cast<IdentifierExpr>(expr)) {
            return std::make_shared<AssignExpr>(equals.location, expr, BinaryOp::EQUAL, value);
        }
        error(equals, "Invalid assignment target.");
    }
    
    return expr;
}

Expr::Ptr Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    
    while (match(TokenType::OR)) {
        auto operatorToken = previous();
        auto right = parseLogicalAnd();
        left = std::make_shared<LogicalExpr>(operatorToken.location, left, false, right);
    }
    
    return left;
}

Expr::Ptr Parser::parseLogicalAnd() {
    auto left = parseBitwiseOr();
    
    while (match(TokenType::AND)) {
        auto operatorToken = previous();
        auto right = parseBitwiseOr();
        left = std::make_shared<LogicalExpr>(operatorToken.location, left, true, right);
    }
    
    return left;
}

Expr::Ptr Parser::parseBitwiseOr() {
    return parseBitwiseXor();
}

Expr::Ptr Parser::parseBitwiseXor() {
    return parseBitwiseAnd();
}

Expr::Ptr Parser::parseBitwiseAnd() {
    return parseEquality();
}

Expr::Ptr Parser::parseEquality() {
    auto left = parseComparison();
    
    while (match(TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL)) {
        auto operatorToken = previous();
        auto right = parseComparison();
        auto op = (operatorToken.type == TokenType::EQUAL_EQUAL) ? BinaryOp::EQUAL_EQUAL : BinaryOp::BANG_EQUAL;
        left = std::make_shared<BinaryExpr>(operatorToken.location, left, op, right);
    }
    
    return left;
}

Expr::Ptr Parser::parseComparison() {
    auto left = parseShift();
    
    while (match(TokenType::GREATER, TokenType::GREATER_EQUAL, 
                TokenType::LESS, TokenType::LESS_EQUAL)) {
        auto operatorToken = previous();
        auto right = parseShift();
        BinaryOp op;
        switch (operatorToken.type) {
            case TokenType::GREATER: op = BinaryOp::GREATER; break;
            case TokenType::GREATER_EQUAL: op = BinaryOp::GREATER_EQUAL; break;
            case TokenType::LESS: op = BinaryOp::LESS; break;
            case TokenType::LESS_EQUAL: op = BinaryOp::LESS_EQUAL; break;
            default: op = BinaryOp::GREATER; break;
        }
        left = std::make_shared<BinaryExpr>(operatorToken.location, left, op, right);
    }
    
    return left;
}

Expr::Ptr Parser::parseShift() {
    return parseAdditive();
}

Expr::Ptr Parser::parseAdditive() {
    auto left = parseMultiplicative();
    
    while (match(TokenType::PLUS, TokenType::MINUS)) {
        auto operatorToken = previous();
        auto right = parseMultiplicative();
        BinaryOp op = (operatorToken.type == TokenType::PLUS) ? BinaryOp::PLUS : BinaryOp::MINUS;
        left = std::make_shared<BinaryExpr>(operatorToken.location, left, op, right);
    }
    
    return left;
}

Expr::Ptr Parser::parseMultiplicative() {
    auto left = parseUnary();
    
    while (match(TokenType::STAR, TokenType::SLASH, TokenType::PERCENT)) {
        auto operatorToken = previous();
        auto right = parseUnary();
        BinaryOp op;
        switch (operatorToken.type) {
            case TokenType::STAR: op = BinaryOp::STAR; break;
            case TokenType::SLASH: op = BinaryOp::SLASH; break;
            case TokenType::PERCENT: op = BinaryOp::PERCENT; break;
            default: op = BinaryOp::STAR; break;
        }
        left = std::make_shared<BinaryExpr>(operatorToken.location, left, op, right);
    }
    
    return left;
}

Expr::Ptr Parser::parseUnary() {
    if (match(TokenType::BANG, TokenType::MINUS, TokenType::TILDE)) {
        auto operatorToken = previous();
        auto right = parseUnary();
        UnaryOp op;
        switch (operatorToken.type) {
            case TokenType::BANG: op = UnaryOp::BANG; break;
            case TokenType::MINUS: op = UnaryOp::MINUS; break;
            case TokenType::TILDE: op = UnaryOp::TILDE; break;
            default: op = UnaryOp::MINUS; break;
        }
        return std::make_shared<UnaryExpr>(operatorToken.location, op, right);
    }
    
    return parseCall();
}

Expr::Ptr Parser::parseCall() {
    auto expr = parsePrimary();
    if (!expr) return nullptr;  // Prevent infinite loop
    
    while (true) {
        if (match(TokenType::LEFT_PAREN)) {
            expr = std::make_shared<CallExpr>(previous().location, expr, previous(), parseArgumentList());
        } else if (match(TokenType::LEFT_BRACKET)) {
            auto index = parseExpression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index expression.");
            expr = std::make_shared<IndexExpr>(previous().location, expr, index);
        } else if (match(TokenType::DOT)) {
            auto name = advance();
            if (name.type != TokenType::IDENTIFIER) {
                error(name, "Expected property name after '.'.");
                break;  // Don't continue, break instead
            }
            expr = std::make_shared<DotExpr>(previous().location, expr, name.lexeme);
        } else {
            break;
        }
    }
    
    return expr;
}

Expr::Ptr Parser::parsePrimary() {
    if (match(TokenType::FALSE_KEYWORD)) {
        return std::make_shared<LiteralExpr>(previous().location, false, Type::boolean());
    }
    if (match(TokenType::TRUE_KEYWORD)) {
        return std::make_shared<LiteralExpr>(previous().location, true, Type::boolean());
    }
    if (match(TokenType::NULL_KEYWORD)) {
        return std::make_shared<LiteralExpr>(previous().location, nullptr, Type::null());
    }
    if (match(TokenType::UNDEFINED_KEYWORD)) {
        return std::make_shared<LiteralExpr>(previous().location, Value(), Type::undefined());
    }
    if (match(TokenType::SUPER_KEYWORD)) {
        return std::make_shared<SuperExpr>(previous().location, "super", std::vector<Expr::Ptr>{});
    }
    if (match(TokenType::THIS_KEYWORD)) {
        return std::make_shared<ThisExpr>(previous().location);
    }
    
    if (match(TokenType::NUMBER)) {
        return std::make_shared<LiteralExpr>(previous().location, previous().literal.numberValue, Type::number());
    }
    if (match(TokenType::STRING)) {
        return std::make_shared<LiteralExpr>(previous().location, previous().literal.stringValue, Type::string());
    }
    
    if (match(TokenType::IDENTIFIER)) {
        return std::make_shared<IdentifierExpr>(previous().location, previous().lexeme);
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_shared<GroupingExpr>(previous().location, expr);
    }
    
    if (match(TokenType::LEFT_BRACKET)) {
        auto elements = parseCommaSeparatedExpressions();
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
        return std::make_shared<ArrayLiteralExpr>(previous().location, elements);
    }
    
    if (match(TokenType::LEFT_BRACE)) {
        if (check(TokenType::RIGHT_BRACE)) {
            advance();
            return std::make_shared<ObjectLiteralExpr>(previous().location, std::vector<ObjectLiteralExpr::Property>{});
        }
        
        std::vector<ObjectLiteralExpr::Property> properties;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            ObjectLiteralExpr::Property prop;
            auto key = advance();
            
            if (key.type == TokenType::IDENTIFIER) {
                prop.name = key.lexeme;
                if (match(TokenType::COLON)) {
                    prop.value = parseExpression();
                } else {
                    // Method shorthand
                    prop.isMethod = true;
                    prop.value = std::dynamic_pointer_cast<Expr>(parseFunction());
                }
            } else if (key.type == TokenType::STRING) {
                prop.name = key.literal.stringValue;
                consume(TokenType::COLON, "Expect ':' after property name.");
                prop.value = parseExpression();
            }
            
            properties.push_back(prop);
            
            if (!match(TokenType::COMMA)) break;
        }
        
        consume(TokenType::RIGHT_BRACE, "Expect '}' after object properties.");
        return std::make_shared<ObjectLiteralExpr>(previous().location, properties);
    }
    
    error(peek(), "Expected expression.");
    return std::make_shared<LiteralExpr>(previous().location, 0.0);  // Return dummy value
}

std::vector<Expr::Ptr> Parser::parseArgumentList() {
    std::vector<Expr::Ptr> args;
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            args.push_back(parseExpression());
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    return args;
}

std::vector<Expr::Ptr> Parser::parseCommaSeparatedExpressions() {
    std::vector<Expr::Ptr> exprs;
    
    if (!check(TokenType::RIGHT_BRACKET)) {
        do {
            exprs.push_back(parseExpression());
        } while (match(TokenType::COMMA));
    }
    
    return exprs;
}

Type Parser::parseType() {
    if (match(TokenType::IDENTIFIER)) {
        return Type::of(previous().lexeme);
    }
    return Type::undefined();
}

std::string Parser::parseTypeAnnotation() {
    if (check(TokenType::IDENTIFIER)) {
        advance();
        return previous().lexeme;
    }
    return "";
}

Program parse(const std::string& source, const std::string& filename) {
    Lexer lexer(source, filename);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    return parser.parse();
}

} // namespace hs
