#include "lexer.h"
#include <cctype>
#include <cstdlib>

namespace hs {

Lexer::Lexer(const std::string& src, const std::string& filename)
    : source(src), filename(filename), buffer(src) {}

std::vector<Token> Lexer::tokenize() {
    while (buffer.hasNext()) {
        scanToken();
    }
    addToken(TokenType::EOF_TOKEN);
    return tokens;
}

Result<std::vector<Token>> Lexer::tokenizeWithErrorHandling() {
    auto result = tokenize();
    if (hasErrors()) {
        return Result<std::vector<Token>>(*lastError);
    }
    return Result<std::vector<Token>>(std::move(result));
}

void Lexer::scanToken() {
    // Skip whitespace
    while (buffer.hasNext() && (buffer.peek() == ' ' || buffer.peek() == '\t' || 
           buffer.peek() == '\r' || buffer.peek() == '\n')) {
        buffer.advance();
    }
    
    if (!buffer.hasNext()) {
        return;  // EOF
    }
    
    char c = buffer.advance();
    
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case '[': addToken(TokenType::LEFT_BRACKET); break;
        case ']': addToken(TokenType::RIGHT_BRACKET); break;
        case ',': addToken(TokenType::COMMA); break;
      case '.':
             if (buffer.check('?')) {
                 buffer.advance();
                 addToken(TokenType::DOT_QUESTION);
             } else if (buffer.check('.')) {
                 buffer.advance();
                 if (buffer.check('.')) {
                     buffer.advance();
                     addToken(TokenType::DOT_DOT_DOT);
                 } else {
                     addToken(TokenType::DOT_DOT);
                 }
             } else {
                 addToken(TokenType::DOT);
             }
             break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '!':
            if (buffer.match('=')) addToken(TokenType::BANG_EQUAL);
            else addToken(TokenType::BANG);
            break;
        case '=':
            if (buffer.match('=')) addToken(TokenType::EQUAL_EQUAL);
            else if (buffer.match('>')) addToken(TokenType::ARROW);
            else addToken(TokenType::EQUAL);
            break;
        case '+':
            if (buffer.match('+')) addToken(TokenType::PLUS_PLUS);
            else if (buffer.match('=')) addToken(TokenType::PLUS_EQUAL);
            else addToken(TokenType::PLUS);
            break;
        case '-':
            if (buffer.match('-')) addToken(TokenType::MINUS_MINUS);
            else if (buffer.match('=')) addToken(TokenType::MINUS_EQUAL);
            else addToken(TokenType::MINUS);
            break;
        case '*':
            if (buffer.match('=')) addToken(TokenType::STAR_EQUAL);
            else addToken(TokenType::STAR);
            break;
        case '/':
            if (buffer.match('=')) addToken(TokenType::SLASH_EQUAL);
            else {
                skipComment();
                if (buffer.hasNext()) scanToken();
            }
            break;
        case '%':
            if (buffer.match('=')) addToken(TokenType::PERCENT_EQUAL);
            else addToken(TokenType::PERCENT);
            break;
        case '<':
            if (buffer.match('<')) addToken(TokenType::LESS_LESS);
            else if (buffer.match('=')) addToken(TokenType::LESS_EQUAL);
            else addToken(TokenType::LESS);
            break;
        case '>':
            if (buffer.match('>')) addToken(TokenType::GREATER_GREATER);
            else if (buffer.match('=')) addToken(TokenType::GREATER_EQUAL);
            else addToken(TokenType::GREATER);
            break;
        case '?':
                 if (buffer.match('?')) {
                     addToken(TokenType::NULLISH_COALESCE);
                 } else {
                     addToken(TokenType::QUESTION);
                 }
                 break;
        case ':': addToken(TokenType::COLON); break;
        case '|':
            if (buffer.match('|')) addToken(TokenType::OR);
            else addToken(TokenType::PIPE);
            break;
        case '&':
            if (buffer.match('&')) addToken(TokenType::AND);
            else addToken(TokenType::AMPERSAND);
            break;
        case '^': addToken(TokenType::CARET); break;
        case '~':
            if (buffer.match('=')) addToken(TokenType::TILDE_EQUALS);
            else addToken(TokenType::TILDE);
            break;
        case '@': addToken(TokenType::AT); break;
        case '"': scanString('"'); break;
        case '\'': scanString('\''); break;
        case '`': scanString('`'); break;
        default:
            if (isDigit(c)) {
                buffer.rewind(1);
                scanNumber();
            } else if (isAlpha(c)) {
                buffer.rewind(1);
                scanIdentifier();
            } else {
                reportError(LexerError::Kind::UnexpectedCharacter, 
                    std::string("unexpected character: '") + c + "'");
            }
            break;
    }
}

void Lexer::scanString(char quote) {
    std::string content;
    bool isTemplate = (quote == '`');
    
    while (buffer.hasNext()) {
        if (buffer.check(quote) && !isTemplate) {
            buffer.advance();  // consume closing quote
            break;
        }
        
        if (isTemplate && buffer.check(quote)) {
            content += buffer.advance();
            continue;
        }
        
        char c = buffer.advance();
        
        if (c == '\\' && buffer.hasNext()) {
            char next = buffer.advance();
            switch (next) {
                case 'n': content += '\n'; break;
                case 't': content += '\t'; break;
                case 'r': content += '\r'; break;
                case 'b': content += '\b'; break;
                case 'f': content += '\f'; break;
                case 'v': content += '\v'; break;
                case '\\': content += '\\'; break;
                case '"': content += '"'; break;
                case '\'': content += '\''; break;
                case '`': content += '`'; break;
                case '0': content += '\0'; break;
                case 'u': {
                    if (buffer.check('{')) {
                        buffer.advance();
                        std::string hex = buffer.takeWhile([](char c){ return std::isxdigit(c); });
                        if (hex.length() > 4) hex = hex.substr(0, 4);
                        if (buffer.check('}')) buffer.advance();
                        unsigned int codepoint = std::stoul(hex, nullptr, 16);
                        if (codepoint < 0x80) {
                            content += static_cast<char>(codepoint);
                        } else if (codepoint < 0x800) {
                            content += static_cast<char>(0xC0 | (codepoint >> 6));
                            content += static_cast<char>(0x80 | (codepoint & 0x3F));
                        } else {
                            content += static_cast<char>(0xE0 | (codepoint >> 12));
                            content += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                            content += static_cast<char>(0x80 | (codepoint & 0x3F));
                        }
                    } else {
                        std::string hex = buffer.takeN(4);
                        if (hex.length() < 4) {
                            reportError(LexerError::Kind::InvalidUnicodeEscape, 
                                "unicode escape must have 4 hex digits");
                        } else {
                            unsigned int codepoint = std::stoul(hex, nullptr, 16);
                            content += static_cast<char>(codepoint);
                        }
                    }
                    break;
                }
                case 'x': {
                    std::string hex = buffer.takeN(2);
                    content += static_cast<char>(std::stoul(hex, nullptr, 16));
                    break;
                }
                default: {
                    if (!isTemplate) {
                        reportError(LexerError::Kind::InvalidEscapeSequence, 
                            std::string("invalid escape sequence: \\") + next);
                    }
                    content += next;
                    break;
                }
            }
        } else if (isTemplate && c == '{') {
            content += c;
        } else {
            if (c == '\n') {
                reportError(LexerError::Kind::UnterminatedString, 
                    "string cannot contain newline");
            }
            content += c;
        }
    }
    
    if (!isTemplate && !buffer.check(quote) && !buffer.hasNext()) {
        reportError(LexerError::Kind::UnterminatedString, "unterminated string");
    }
    
    addToken(TokenType::STRING, content);
}

void Lexer::scanNumber() {
    auto& self = *this;
    std::string integerPart = buffer.takeWhile([&self](char c){ return self.isDigit(c); });
    
    if (buffer.check('.') && isDigit(buffer.peekNext())) {
        buffer.advance();
        std::string fractional = buffer.takeWhile([&self](char c){ return self.isDigit(c); });
        integerPart += '.' + fractional;
    }
    
    if (buffer.check('e') || buffer.check('E')) {
        integerPart += buffer.advance();
        if (buffer.check('+') || buffer.check('-')) {
            integerPart += buffer.advance();
        }
        integerPart += buffer.takeWhile([&self](char c){ return self.isDigit(c); });
    }
    
    double value = std::stod(integerPart);
    addNumberToken(value);
}

void Lexer::scanIdentifier() {
    auto& self = *this;
    std::string lexeme = buffer.takeWhile([&self](char c){ return self.isAlphaNum(c); });
    
    TokenType type = KeywordTable::get(lexeme);
    if (type == TokenType::IDENTIFIER) {
        addToken(TokenType::IDENTIFIER, lexeme);
    } else {
        addToken(type, lexeme);
    }
}

void Lexer::skipComment() {
    if (buffer.match('/')) {
        skipLineComment();
    } else if (buffer.match('*')) {
        skipBlockComment();
    } else {
        addToken(TokenType::SLASH);
    }
}

void Lexer::skipLineComment() {
    while (buffer.peek() != '\n' && buffer.hasNext()) {
        buffer.advance();
    }
}

void Lexer::skipBlockComment() {
    bool endFound = false;
    while (buffer.hasNext()) {
        if (buffer.check('*') && buffer.peekNext() == '/') {
            buffer.advance();
            buffer.advance();
            endFound = true;
            break;
        }
        buffer.advance();
    }
    if (!endFound) {
        reportError(LexerError::Kind::UnterminatedString, 
            "unterminated block comment");
    }
}

void Lexer::addToken(TokenType type, const std::string& lexeme) {
    SourceLocation loc = buffer.location();
    loc.line -= 1;
    loc.column -= static_cast<int>(lexeme.length());
    if (type == TokenType::STRING) {
        tokens.push_back(Token(type, lexeme, TokenLiteral(lexeme), loc));
    } else {
        tokens.push_back(Token(type, lexeme, loc));
    }
}

void Lexer::addToken(TokenType type) {
    SourceLocation loc = buffer.location();
    tokens.push_back(Token(type, loc));
}

void Lexer::addNumberToken(double value) {
    SourceLocation loc = buffer.location();
    loc.line -= 1;
    tokens.push_back(Token(TokenType::NUMBER, std::to_string(value), TokenLiteral(value), loc));
}

void Lexer::reportError(LexerError::Kind kind, const std::string& detail) {
    SourceLocation loc = buffer.location();
    LexerError err(loc, kind, detail);
    errors.push_back(err);
    lastError = err;
}

std::vector<Token> tokenize(const std::string& source, const std::string& filename) {
    Lexer lexer(source, filename);
    return lexer.tokenize();
}

} // namespace hs
