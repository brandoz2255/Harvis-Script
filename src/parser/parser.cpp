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
        if (match(TokenType::STRUCT_KEYWORD)) return parseStruct();
        if (match(TokenType::INTERFACE_KEYWORD)) return parseInterface();
        if (match(TokenType::FUNCTION_KEYWORD)) return parseFunction(true);
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
        if (match(TokenType::DEFER_KEYWORD)) return parseDefer();
        if (match(TokenType::GO_KEYWORD)) return parseGo();
        if (match(TokenType::SELECT_KEYWORD)) return parseSelect();
        if (match(TokenType::IMPORT_KEYWORD)) return parseImport();
        if (match(TokenType::EXPORT_KEYWORD)) return parseExport();
        if (match(TokenType::TRY_KEYWORD)) return parseTry();
        if (match(TokenType::THROW_KEYWORD)) return parseThrow();
        if (match(TokenType::SWITCH_KEYWORD)) return parseSwitch();
        if (match(TokenType::RANGE_KEYWORD)) return parseRange();
        if (match(TokenType::LEFT_BRACE)) return parseBlockStatement();
        
        return parseExpressionStatement();
    } catch (const std::exception& e) {
        synchronize();
        return nullptr;
    }
}

 Stmt::Ptr Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    match(TokenType::SEMICOLON);  // Optional semicolon
    return std::make_shared<ExpressionStmt>(previous().location, expr);
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
    
    std::vector<std::string> typeParams;
    if (match(TokenType::LESS)) {
        while (!check(TokenType::GREATER) && !isAtEnd()) {
            auto param = advance();
            if (param.type == TokenType::IDENTIFIER) {
                typeParams.push_back(param.lexeme);
            }
            if (!match(TokenType::COMMA)) break;
        }
        consume(TokenType::GREATER, "Expect '>' after type parameters.");
    }
    
    consume(TokenType::LEFT_BRACE, "Expect '{' after class name.");
    
    std::vector<std::shared_ptr<FunctionStmt>> methods;
    std::vector<std::shared_ptr<VarDeclStmt>> fields;
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (check(TokenType::FUNCTION_KEYWORD) || check(TokenType::CONSTRUCTOR_KEYWORD)) {
            bool isConstructor = check(TokenType::CONSTRUCTOR_KEYWORD);
            if (isConstructor) advance();
            auto fn = std::dynamic_pointer_cast<FunctionStmt>(parseFunction(!isConstructor));
            if (isConstructor && fn) fn->name = "constructor";
            if (fn) methods.push_back(fn);
        } else {
            auto stmt = parseStatement();
            if (stmt) {
                auto var = std::dynamic_pointer_cast<VarDeclStmt>(stmt);
                if (var) fields.push_back(var);
            }
        }
    }
    
    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
    
    std::vector<Expr::Ptr> emptyExpr;
    std::vector<std::pair<std::string, Type>> emptyType;
    
    return std::make_shared<ClassStmt>(name.location, name.lexeme, emptyExpr, typeParams, emptyType, std::vector<std::string>{}, methods, fields, false, false);
}

Stmt::Ptr Parser::parseFunction(bool expectName) {
    std::string name = "";
    if (match(TokenType::IDENTIFIER)) {
        name = previous().lexeme;
    } else if (expectName) {
        // No identifier found but expected one
        if (!check(TokenType::LEFT_PAREN)) {
            error(previous(), "Expected function name.");
            return nullptr;
        }
    }
    
    SourceLocation loc = previous().location;
    
    // Only parse generic type params if we have a function name (not after constructor keyword)
    std::vector<std::string> typeParams;
    if (!name.empty() && match(TokenType::LESS)) {
        while (!check(TokenType::GREATER) && !isAtEnd()) {
            auto param = advance();
            if (param.type == TokenType::IDENTIFIER) {
                typeParams.push_back(param.lexeme);
            }
            if (!match(TokenType::COMMA)) break;
        }
        if (check(TokenType::GREATER)) {
            advance(); // consume >
        }
    }
    
    // If we didn't match a name and we're at `(`, we're in a constructor context
    if (name.empty() && check(TokenType::LEFT_PAREN)) {
        // Don't try to parse generics, just consume the paren
    }
    
  consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
     auto params = parseParameterList();
     
     Type returnType = Type::undefined();
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
     
     return std::make_shared<FunctionStmt>(loc, name, params, returnType, body, false, false, false, false, std::move(typeParams));
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
            Type paramType;
            if (match(TokenType::COLON)) {
                paramType = parseType();
            }
            params.emplace_back(param.lexeme, paramType);
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
        auto name = advance();
        Type type;
        if (match(TokenType::COLON)) {
            type = parseType();
        }
        Expr::Ptr initializer;
        if (match(TokenType::EQUAL)) {
            initializer = parseAssignment();
        }
        init = std::make_shared<VarDeclStmt>(loc, name.lexeme, type, initializer, false, false);
        match(TokenType::SEMICOLON);
    } else if (match(TokenType::LET_KEYWORD)) {
        auto name = advance();
        Type type;
        if (match(TokenType::COLON)) {
            type = parseType();
        }
        Expr::Ptr initializer;
        if (match(TokenType::EQUAL)) {
            initializer = parseAssignment();
        }
        init = std::make_shared<VarDeclStmt>(loc, name.lexeme, type, initializer, false, true);
        consume(TokenType::SEMICOLON, "Expect ';' after for initializer.");
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
    std::string moduleName;
    std::vector<std::pair<std::string, std::string>> namedImports;
    std::string defaultImport;
    std::string namespaceImport;

    // Parse import specifier(s)
    if (match(TokenType::LEFT_BRACE)) {
        // Named imports: { name1, name2 } or { name1 as alias1 }
        do {
            advance();
            std::string name = previous().lexeme;
            if (match(TokenType::AS_KEYWORD)) {
                advance();
                name = previous().lexeme;
            }
            namedImports.push_back({name, name});
        } while (match(TokenType::COMMA));
        consume(TokenType::RIGHT_BRACE, "Expect '}' after named imports.");
    } else if (match(TokenType::STAR)) {
        // Namespace import: * as name
        consume(TokenType::AS_KEYWORD, "Expect 'as' after '*' in import.");
        consume(TokenType::IDENTIFIER, "Expect namespace name.");
        namespaceImport = previous().lexeme;
    } else if (check(TokenType::IDENTIFIER)) {
        // Default import: name
        advance();
        defaultImport = previous().lexeme;
    }

    // Parse 'from' clause
    if (match(TokenType::FROM_KEYWORD)) {
        consume(TokenType::STRING, "Expect module name string after 'from'.");
        moduleName = previous().literal.stringValue;
    } else if (check(TokenType::STRING)) {
        // Shorthand: import "module"
        advance();
        moduleName = previous().literal.stringValue;
    } else {
        error(previous(), "Expect 'from' clause or module name.");
    }

    consume(TokenType::SEMICOLON, "Expect ';' after import statement.");
    return std::make_shared<ImportStmt>(loc, moduleName, namedImports, defaultImport, namespaceImport);
}

Stmt::Ptr Parser::parseExport() {
    SourceLocation loc = previous().location;

    // Check for named exports: export { name1, name2 }
    if (match(TokenType::LEFT_BRACE)) {
        std::vector<std::string> names;
        do {
            consume(TokenType::IDENTIFIER, "Expect identifier in export list.");
            names.push_back(previous().lexeme);
        } while (match(TokenType::COMMA));
        consume(TokenType::RIGHT_BRACE, "Expect '}' after export list.");
        
        std::string allFrom;
        if (match(TokenType::FROM_KEYWORD)) {
            consume(TokenType::STRING, "Expect module name after 'from'.");
            allFrom = previous().literal.stringValue;
        }
        consume(TokenType::SEMICOLON, "Expect ';' after export statement.");
        return std::make_shared<ExportStmt>(loc, false, Expr::Ptr(nullptr), names, allFrom);
    }

    // Check for default export: export default ...
    if (match(TokenType::DEFAULT_KEYWORD)) {
        if (match(TokenType::FUNCTION_KEYWORD)) {
            auto func = parseFunction(true);
            return std::make_shared<ExportStmt>(loc, true,
                std::static_pointer_cast<FunctionStmt>(func));
        } else if (match(TokenType::CLASS_KEYWORD)) {
            auto cls = parseClass();
            return std::make_shared<ExportStmt>(loc, true,
                std::static_pointer_cast<ClassStmt>(cls));
        } else {
            auto expr = parseExpression();
            consume(TokenType::SEMICOLON, "Expect ';' after export statement.");
            return std::make_shared<ExportStmt>(loc, true, expr);
        }
    }

    // export const/let/var ...
    if (match(TokenType::CONST_KEYWORD)) {
        auto var = parseConst();
        return std::make_shared<ExportStmt>(loc, false,
            std::static_pointer_cast<VarDeclStmt>(var));
    }
    if (match(TokenType::LET_KEYWORD)) {
        auto var = parseVariable(true);
        return std::make_shared<ExportStmt>(loc, false,
            std::static_pointer_cast<VarDeclStmt>(var));
    }
    if (match(TokenType::VAR_KEYWORD)) {
        auto var = parseVariable(false);
        return std::make_shared<ExportStmt>(loc, false,
            std::static_pointer_cast<VarDeclStmt>(var));
    }

    // export function ...
    if (match(TokenType::FUNCTION_KEYWORD)) {
        auto func = parseFunction(true);
        return std::make_shared<ExportStmt>(loc, false,
            std::static_pointer_cast<FunctionStmt>(func));
    }

    // export class ...
    if (match(TokenType::CLASS_KEYWORD)) {
        auto cls = parseClass();
        return std::make_shared<ExportStmt>(loc, false,
            std::static_pointer_cast<ClassStmt>(cls));
    }

    error(previous(), "Expect exported declaration after 'export'.");
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
        
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            if (check(TokenType::CASE_KEYWORD) || check(TokenType::DEFAULT_KEYWORD)) {
                break;
            }
            auto stmt = parseStatement();
            if (stmt) cs.statements.push_back(stmt);
        }
        
        cases.push_back(cs);
    }
    
    consume(TokenType::RIGHT_BRACE, "Expect '}' after switch statement.");
    return std::make_shared<SwitchStmt>(loc, expression, cases);
}

Stmt::Ptr Parser::parseRange() {
    SourceLocation loc = previous().location;
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'range'.");
    
    std::vector<std::string> variables;
    if (check(TokenType::IDENTIFIER)) {
        variables.push_back(advance().lexeme);
        if (match(TokenType::COMMA)) {
            if (check(TokenType::IDENTIFIER)) {
                variables.push_back(advance().lexeme);
            }
        }
    } else {
        error(peek(), "Expect variable name(s) after 'range'.");
    }
    
    consume(TokenType::IN_KEYWORD, "Expect 'in' after variable name(s).");
    auto collection = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after collection.");
    
    auto body = parseStatement();
    return std::make_shared<RangeStmt>(loc, variables, collection, body);
}

Stmt::Ptr Parser::parseTypeSwitch() {
    SourceLocation loc = previous().location;
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'type switch'.");
    auto expression = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after type switch expression.");
    
    consume(TokenType::LEFT_BRACE, "Expect '{' before type switch cases.");
    
    std::vector<TypeCase> cases;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (match(TokenType::CASE_KEYWORD)) {
            Type caseType = parseType();
            consume(TokenType::COLON, "Expect ':' after type in type switch case.");
            
            TypeCase tc(loc, caseType, std::vector<Stmt::Ptr>{});
            while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
                if (check(TokenType::CASE_KEYWORD)) {
                    break;
                }
                auto stmt = parseStatement();
                if (stmt) tc.statements.push_back(stmt);
            }
            cases.push_back(tc);
        } else if (match(TokenType::DEFAULT_KEYWORD)) {
            consume(TokenType::COLON, "Expect ':' after 'default' in type switch.");
            TypeCase tc(loc, Type::undefined(), std::vector<Stmt::Ptr>{});
            while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
                auto stmt = parseStatement();
                if (stmt) tc.statements.push_back(stmt);
            }
            cases.push_back(tc);
        } else {
            break;
        }
    }
    
    consume(TokenType::RIGHT_BRACE, "Expect '}' after type switch statement.");
    return std::make_shared<TypeSwitchStmt>(loc, expression, cases, std::vector<Stmt::Ptr>{});
}

// ============ EXPRESSION PARSING ============

Expr::Ptr Parser::parseExpression() {
    return parseAssignment();
}

Expr::Ptr Parser::parseAssignment() {
    auto expr = parseLogicalOr();
    
    if (match(TokenType::EQUAL)) {
        auto equals = previous();
        auto value = parseAssignment();
        
        // Check if we can assign to this (IdentifierExpr or DotExpr like this.name)
        if (auto ident = std::dynamic_pointer_cast<IdentifierExpr>(expr)) {
            return std::make_shared<AssignExpr>(equals.location, expr, BinaryOp::EQUAL, value);
        }
        if (auto dot = std::dynamic_pointer_cast<DotExpr>(expr)) {
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
     auto left = parseNullishCoalescing();
     
     while (match(TokenType::AND)) {
         auto operatorToken = previous();
         auto right = parseNullishCoalescing();
         left = std::make_shared<LogicalExpr>(operatorToken.location, left, true, right);
     }
     
     return left;
 }
 
 Expr::Ptr Parser::parseNullishCoalescing() {
     auto left = parseBitwiseOr();
     
     while (match(TokenType::NULLISH_COALESCE)) {
         auto operatorToken = previous();
         auto right = parseBitwiseOr();
         left = std::make_shared<BinaryExpr>(operatorToken.location, left, BinaryOp::NULLISH_COALESCE, right);
     }
     
     return left;
 }

 Expr::Ptr Parser::parseBitwiseOr() {
    auto left = parseBitwiseXor();
    
    while (match(TokenType::PIPE)) {
        auto operatorToken = previous();
        auto right = parseBitwiseXor();
        left = std::make_shared<BinaryExpr>(operatorToken.location, left, BinaryOp::PIPE, right);
    }
    
    return left;
}

Expr::Ptr Parser::parseBitwiseXor() {
    auto left = parseBitwiseAnd();
    
    while (match(TokenType::CARET)) {
        auto operatorToken = previous();
        auto right = parseBitwiseAnd();
        left = std::make_shared<BinaryExpr>(operatorToken.location, left, BinaryOp::CARET, right);
    }
    
    return left;
}

Expr::Ptr Parser::parseBitwiseAnd() {
    auto left = parseEquality();
    
    while (match(TokenType::AMPERSAND)) {
        auto operatorToken = previous();
        auto right = parseEquality();
        left = std::make_shared<BinaryExpr>(operatorToken.location, left, BinaryOp::AMPERSAND, right);
    }
    
    return left;
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
    auto left = parseAdditive();
    
    while (match(TokenType::LESS_LESS, TokenType::GREATER_GREATER)) {
        auto operatorToken = previous();
        auto right = parseAdditive();
        BinaryOp op;
        if (operatorToken.type == TokenType::LESS_LESS) {
            op = BinaryOp::LESS_LESS;
        } else {
            op = BinaryOp::GREATER_GREATER;
        }
        left = std::make_shared<BinaryExpr>(operatorToken.location, left, op, right);
    }
    
    return left;
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
             std::vector<Type> typeArgs;
             if (check(TokenType::LESS)) {
                 advance();
                 while (!check(TokenType::GREATER) && !isAtEnd()) {
                     typeArgs.push_back(parseType());
                     if (!match(TokenType::COMMA)) break;
                 }
                 consume(TokenType::GREATER, "Expect '>' after type arguments.");
             }
             auto args = parseArgumentList();
             expr = std::make_shared<CallExpr>(previous().location, expr, previous(), std::move(args), std::move(typeArgs));
         } else if (match(TokenType::LEFT_BRACKET)) {
             bool optional = false;
             if (match(TokenType::DOT_QUESTION)) {
                 optional = true;
             } else {
                 // Put back the DOT token
                 // Actually we need to check for ?[ before [
             }
             // Re-check for the bracket
             if (!match(TokenType::LEFT_BRACKET)) {
                 // This shouldn't happen, but handle gracefully
                 break;
             }
             auto index = parseExpression();
             consume(TokenType::RIGHT_BRACKET, "Expect ']' after index expression.");
             expr = std::make_shared<IndexExpr>(previous().location, expr, index, optional);
         } else if (match(TokenType::DOT)) {
             bool optional = false;
             if (match(TokenType::QUESTION)) {
                 optional = true;
                 // Now expect an identifier or LEFT_PAREN
                 if (check(TokenType::IDENTIFIER)) {
                     auto name = advance();
                     expr = std::make_shared<DotExpr>(previous().location, expr, name.lexeme, true);
                 } else if (match(TokenType::LEFT_PAREN)) {
                     auto args = parseArgumentList();
                     expr = std::make_shared<CallExpr>(previous().location, expr, previous(), std::move(args), std::vector<Type>{});
                 } else {
                     error(peek(), "Expected property name or '(' after '?.'?");
                     break;
                 }
             } else {
                  // Check if next token is LEFT_BRACKET (e.g., arr.[index])
                  if (check(TokenType::LEFT_BRACKET)) {
                      advance(); // consume '['
                      auto index = parseExpression();
                      consume(TokenType::RIGHT_BRACKET, "Expect ']' after index expression.");
                      expr = std::make_shared<IndexExpr>(previous().location, expr, index, false);
                  } else {
                      auto name = advance();
                      if (name.type != TokenType::IDENTIFIER) {
                          error(name, "Expected property name after '.'.");
                          break;
                      }
                      expr = std::make_shared<DotExpr>(previous().location, expr, name.lexeme, false);
                  }
              }
          } else if (match(TokenType::DOT_QUESTION)) {
             // Optional chaining: obj?.prop or obj?.()
             if (check(TokenType::IDENTIFIER)) {
                 auto name = advance();
                 expr = std::make_shared<DotExpr>(previous().location, expr, name.lexeme, true);
             } else if (match(TokenType::LEFT_PAREN)) {
                 auto args = parseArgumentList();
                 expr = std::make_shared<CallExpr>(previous().location, expr, previous(), std::move(args), std::vector<Type>{});
             } else {
                 error(peek(), "Expected property name or '(' after '?.'?");
                 break;
             }
} else if (check(TokenType::LESS) && peekNext().type == TokenType::MINUS) {
             advance();
             advance();  // Consume '<-'
             expr = parseSend(std::move(expr));
         } else if (match(TokenType::AS_KEYWORD)) {
             // Type assertion: expr as Type
             Type assertType = parseType();
             expr = std::make_shared<TypeAssertExpr>(previous().location, expr, assertType);
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
    if (match(TokenType::PANIC_KEYWORD)) {
        return parsePanic();
    }
    if (match(TokenType::RECOVER_KEYWORD)) {
        return parseRecover();
    }
    if (match(TokenType::CHANNEL_KEYWORD)) {
        return parseChannel();
    }
    if (match(TokenType::MUTEX_KEYWORD)) {
        return parseMutex();
    }
    if (match(TokenType::WAITGROUP_KEYWORD)) {
        return parseWaitGroup();
    }
    if (check(TokenType::LESS) && peekNext().type == TokenType::MINUS) {
        advance();  // Consume '<'
        advance();  // Consume '-'
        return parseReceive();
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
    
   if (match(TokenType::NEW_KEYWORD)) {
        return parseStructInstantiation();
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
    
    advance();  // Advance past the unknown token
    error(previous(), "Expected expression.");
    if (current > 0) {
        return std::make_shared<LiteralExpr>(previous().location, 0.0);
    }
    return std::make_shared<LiteralExpr>(tokens[0].location, 0.0);  // Return dummy value
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
        std::string name = previous().lexeme;
        std::vector<Type> params;
        if (match(TokenType::LESS)) {
            while (!check(TokenType::GREATER) && !isAtEnd()) {
                params.push_back(parseType());
                if (!match(TokenType::COMMA)) break;
            }
            consume(TokenType::GREATER, "Expect '>' after type arguments.");
        }
        Type result = Type::of(name);
        if (!params.empty()) {
            result.params = std::move(params);
        }
        return result;
    }
    return Type::undefined();
}

std::vector<Type> Parser::parseTypeArgs() {
    std::vector<Type> args;
    if (!check(TokenType::GREATER) && !isAtEnd()) {
        do {
            args.push_back(parseType());
        } while (match(TokenType::COMMA));
    }
    return args;
}

Type Parser::parseTypeAnnotation() {
    if (check(TokenType::IDENTIFIER)) {
        return parseType();
    }
    return Type::undefined();
}

std::vector<std::pair<std::string, Type>> Parser::parseFieldList() {
    std::vector<std::pair<std::string, Type>> fields;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        auto name = advance();
        if (name.type != TokenType::IDENTIFIER) {
            error(name, "Expected field name.");
            continue;
        }
        Type type;
        if (match(TokenType::COLON)) {
            type = parseType();
        } else {
            type = Type::undefined();
        }
        fields.emplace_back(name.lexeme, type);
        if (!match(TokenType::SEMICOLON) && !match(TokenType::COMMA)) {
            break;
        }
    }
    return fields;
}

Stmt::Ptr Parser::parseStruct() {
    // Struct keyword already consumed by parseStatement()
    auto name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        error(name, "Expect struct name.");
        return nullptr;
    }
    std::vector<std::string> typeParams;
    if (match(TokenType::LESS)) {
        while (!check(TokenType::GREATER) && !isAtEnd()) {
            auto param = advance();
            if (param.type == TokenType::IDENTIFIER) {
                typeParams.push_back(param.lexeme);
            }
            if (!match(TokenType::COMMA)) break;
        }
        consume(TokenType::GREATER, "Expect '>' after type parameters.");
    }
    consume(TokenType::LEFT_BRACE, "Expect '{' after struct name.");
    auto fields = parseFieldList();
    consume(TokenType::RIGHT_BRACE, "Expect '}' after struct fields.");
    SourceLocation loc = name.location;
    return std::make_shared<StructDeclStmt>(loc, name.lexeme, std::move(fields), std::move(typeParams));
}

Stmt::Ptr Parser::parseInterface() {
    // Interface keyword already consumed by parseStatement()
    auto name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        error(name, "Expect interface name.");
        return nullptr;
    }
    consume(TokenType::LEFT_BRACE, "Expect '{' after interface name.");
    
    struct InterfaceDeclStmt::Method method;
    std::vector<InterfaceDeclStmt::Method> methods;
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        auto methodName = advance();
        if (methodName.type != TokenType::IDENTIFIER) {
            error(methodName, "Expected method name.");
            break;
        }
        method.name = methodName.lexeme;
        consume(TokenType::LEFT_PAREN, "Expect '(' after method name.");
        method.params = parseParameterList();
        if (match(TokenType::COLON)) {
            method.returnType = parseType();
        } else {
            method.returnType = Type::undefined();
        }
        if (!match(TokenType::SEMICOLON) && !match(TokenType::COMMA)) {
            break;
        }
        methods.push_back(method);
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after interface methods.");
    SourceLocation loc = name.location;
    return std::make_shared<InterfaceDeclStmt>(loc, name.lexeme, std::move(methods));
}

Expr::Ptr Parser::parseStructInstantiation() {
    // New keyword already consumed by parsePrimary()
    auto name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        error(name, "Expect struct type name after 'new'.");
        return nullptr;
    }
    std::vector<Type> typeArgs;
    if (match(TokenType::LESS)) {
        typeArgs = parseTypeArgs();
        consume(TokenType::GREATER, "Expect '>' after type arguments.");
    }
    consume(TokenType::LEFT_BRACE, "Expect '{' after struct type name.");
    
    std::vector<StructInstantiationExpr::Field> fields;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        StructInstantiationExpr::Field field;
        auto fieldName = advance();
        if (fieldName.type != TokenType::IDENTIFIER) {
            error(fieldName, "Expected field name.");
            break;
        }
        field.name = fieldName.lexeme;
        consume(TokenType::COLON, "Expect ':' after field name.");
        field.value = parseExpression();
        fields.push_back(field);
        if (!match(TokenType::COMMA)) break;
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after struct fields.");
    SourceLocation loc = name.location;
    return std::make_shared<StructInstantiationExpr>(loc, name.lexeme, std::move(fields), std::move(typeArgs));
}

Stmt::Ptr Parser::parseDefer() {
    // Defer keyword already consumed by parseStatement()
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after defer expression.");
    SourceLocation loc = previous().location;
    return std::make_shared<DeferStmt>(loc, std::move(expr));
}

Expr::Ptr Parser::parsePanic() {
    // Panic keyword already consumed by parsePrimary()
    auto value = parseExpression();
    SourceLocation loc = previous().location;
    return std::make_shared<PanicExpr>(loc, std::move(value));
}

Expr::Ptr Parser::parseRecover() {
    // Recover keyword already consumed by parsePrimary()
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'recover'.");
    consume(TokenType::RIGHT_PAREN, "Expect ')' after 'recover'.");
    SourceLocation loc = previous().location;
    return std::make_shared<RecoverExpr>(loc);
}

Stmt::Ptr Parser::parseGo() {
    // Go keyword already consumed by parseStatement()
    auto expr = parseExpression();
    SourceLocation loc = previous().location;
    consume(TokenType::SEMICOLON, "Expect ';' after go statement.");
    return std::make_shared<GoStmt>(loc, expr, std::vector<Expr::Ptr>{});
}

Expr::Ptr Parser::parseChannel() {
    // Channel keyword already consumed by parsePrimary()
    Expr::Ptr capacity;
    if (check(TokenType::NUMBER)) {
        auto capExpr = parseExpression();
        capacity = std::move(capExpr);
    }
    SourceLocation loc = previous().location;
    return std::make_shared<ChannelExpr>(loc, std::move(capacity));
}

Expr::Ptr Parser::parseSend(Expr::Ptr channel) {
    // LEFT_ANGLE and MINUS already consumed
    auto value = parseExpression();
    SourceLocation loc = previous().location;
    return std::make_shared<SendExpr>(loc, std::move(channel), std::move(value));
}

Expr::Ptr Parser::parseReceive() {
    // LEFT_ANGLE and MINUS already consumed
    auto channel = parseExpression();
    bool commaOk = match(TokenType::COMMA);
    if (commaOk) {
        auto name = advance();
        if (name.type != TokenType::IDENTIFIER) {
            error(name, "Expected variable name after ',' in receive.");
        }
    }
    SourceLocation loc = previous().location;
    return std::make_shared<ReceiveExpr>(loc, std::move(channel), commaOk);
}

Stmt::Ptr Parser::parseSelect() {
    // Select keyword already consumed by parseStatement()
    consume(TokenType::LEFT_BRACE, "Expect '{' after 'select'.");
    
    std::vector<SelectCase> cases;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        SelectCase cs = parseSelectCase();
        cases.push_back(cs);
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after select body.");
    SourceLocation loc = previous().location;
    return std::make_shared<SelectStmt>(loc, std::move(cases));
}

SelectCase Parser::parseSelectCase() {
    SelectCase cs;
    cs.isSend = false;
    cs.isDefault = false;
    
    if (match(TokenType::DEFAULT_KEYWORD)) {
        cs.isDefault = true;
        consume(TokenType::COLON, "Expect ':' after 'default'.");
    } else if (match(TokenType::CASE_KEYWORD)) {
        cs.channel = parseExpression();
        if (check(TokenType::LESS) && peekNext().type == TokenType::MINUS) {
            advance();
            advance();
            cs.isSend = true;
            cs.value = parseExpression();
        }
        consume(TokenType::COLON, "Expect ':' after select case.");
    } else {
        error(peek(), "Expected 'case' or 'default' in select.");
        return cs;
    }
    
  while (!check(TokenType::CASE_KEYWORD) && !check(TokenType::DEFAULT_KEYWORD) && 
            !check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) cs.body.push_back(stmt);
    }
    
    return cs;
}

Expr::Ptr Parser::parseMutex() {
    // Mutex keyword already consumed by parsePrimary()
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'mutex'.");
    consume(TokenType::RIGHT_PAREN, "Expect ')' after 'mutex'.");
    SourceLocation loc = previous().location;
    return std::make_shared<MutexExpr>(loc);
}

Expr::Ptr Parser::parseWaitGroup() {
    // Waitgroup keyword already consumed by parsePrimary()
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'waitgroup'.");
    consume(TokenType::RIGHT_PAREN, "Expect ')' after 'waitgroup'.");
    SourceLocation loc = previous().location;
    return std::make_shared<WaitGroupExpr>(loc);
}

Program parse(const std::string& source, const std::string& filename) {
    Lexer lexer(source, filename);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    return parser.parse();
}

} // namespace hs
