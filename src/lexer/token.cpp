#include "token.h"
#include "../core/error.h"
#include <unordered_map>

namespace hs {

std::string Token::tokenTypeToString(TokenType t) {
    switch (t) {
        case TokenType::IDENTIFIER: return "identifier";
        case TokenType::STRING: return "string";
        case TokenType::NUMBER: return "number";
        case TokenType::TYPE_KEYWORD: return "type";
        case TokenType::INTERFACE_KEYWORD: return "interface";
        case TokenType::EXTENDS_KEYWORD: return "extends";
        case TokenType::IMPLEMENTS_KEYWORD: return "implements";
        case TokenType::ENUM_KEYWORD: return "enum";
        case TokenType::CONST_KEYWORD: return "const";
        case TokenType::LET_KEYWORD: return "let";
        case TokenType::VAR_KEYWORD: return "var";
        case TokenType::IF_KEYWORD: return "if";
        case TokenType::ELSE_KEYWORD: return "else";
        case TokenType::WHILE_KEYWORD: return "while";
        case TokenType::FOR_KEYWORD: return "for";
        case TokenType::DO_KEYWORD: return "do";
        case TokenType::BREAK_KEYWORD: return "break";
        case TokenType::CONTINUE_KEYWORD: return "continue";
        case TokenType::RETURN_KEYWORD: return "return";
        case TokenType::SWITCH_KEYWORD: return "switch";
        case TokenType::CASE_KEYWORD: return "case";
        case TokenType::DEFAULT_KEYWORD: return "default";
     case TokenType::RANGE_KEYWORD: return "range";
        case TokenType::IN_KEYWORD: return "in";
        case TokenType::STRUCT_KEYWORD: return "struct";
        case TokenType::DEFER_KEYWORD: return "defer";
        case TokenType::PANIC_KEYWORD: return "panic";
        case TokenType::RECOVER_KEYWORD: return "recover";
        case TokenType::GO_KEYWORD: return "go";
        case TokenType::CHANNEL_KEYWORD: return "channel";
        case TokenType::SELECT_KEYWORD: return "select";
        case TokenType::MUTEX_KEYWORD: return "mutex";
        case TokenType::WAITGROUP_KEYWORD: return "waitgroup";
        case TokenType::FUNCTION_KEYWORD: return "function";
        case TokenType::CLASS_KEYWORD: return "class";
        case TokenType::CONSTRUCTOR_KEYWORD: return "constructor";
        case TokenType::NEW_KEYWORD: return "new";
        case TokenType::SUPER_KEYWORD: return "super";
        case TokenType::THIS_KEYWORD: return "this";
        case TokenType::PACKAGE_KEYWORD: return "package";
        case TokenType::EXPORT_KEYWORD: return "export";
        case TokenType::IMPORT_KEYWORD: return "import";
        case TokenType::FROM_KEYWORD: return "from";
        case TokenType::MODULE_KEYWORD: return "module";
        case TokenType::NAMESPACE_KEYWORD: return "namespace";
        case TokenType::AS_KEYWORD: return "as";
        case TokenType::PUBLIC_KEYWORD: return "public";
        case TokenType::PRIVATE_KEYWORD: return "private";
        case TokenType::PROTECTED_KEYWORD: return "protected";
        case TokenType::STATIC_KEYWORD: return "static";
        case TokenType::ABSTRACT_KEYWORD: return "abstract";
        case TokenType::FINAL_KEYWORD: return "final";
        case TokenType::ANY_KEYWORD: return "any";
        case TokenType::UNKNOWN_KEYWORD: return "unknown";
        case TokenType::NEVER_KEYWORD: return "never";
        case TokenType::VOID_KEYWORD: return "void";
        case TokenType::NULL_KEYWORD: return "null";
        case TokenType::TRUE_KEYWORD: return "true";
        case TokenType::FALSE_KEYWORD: return "false";
        case TokenType::UNDEFINED_KEYWORD: return "undefined";
        case TokenType::TRY_KEYWORD: return "try";
        case TokenType::CATCH_KEYWORD: return "catch";
        case TokenType::THROW_KEYWORD: return "throw";
        case TokenType::FINALLY_KEYWORD: return "finally";
        case TokenType::EQUAL: return "=";
        case TokenType::EQUAL_EQUAL: return "==";
        case TokenType::BANG_EQUAL: return "!=";
        case TokenType::LESS: return "<";
        case TokenType::LESS_EQUAL: return "<=";
        case TokenType::GREATER: return ">";
        case TokenType::GREATER_EQUAL: return ">=";
        case TokenType::BANG: return "!";
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::STAR: return "*";
        case TokenType::SLASH: return "/";
        case TokenType::PERCENT: return "%";
        case TokenType::PLUS_PLUS: return "++";
        case TokenType::MINUS_MINUS: return "--";
        case TokenType::PLUS_EQUAL: return "+=";
        case TokenType::MINUS_EQUAL: return "-=";
        case TokenType::STAR_EQUAL: return "*=";
        case TokenType::SLASH_EQUAL: return "/=";
        case TokenType::PERCENT_EQUAL: return "%=";
        case TokenType::AND: return "&&";
        case TokenType::OR: return "||";
        case TokenType::NULLISH_COALESCE: return "??";
        case TokenType::QUESTION: return "?";
        case TokenType::COLON: return ":";
        case TokenType::SEMICOLON: return ";";
        case TokenType::COMMA: return ",";
        case TokenType::DOT: return ".";
        case TokenType::DOT_QUESTION: return "?.?";
        case TokenType::DOT_DOT: return "..";
        case TokenType::DOT_DOT_DOT: return "...";
        case TokenType::ARROW: return "=>";
        case TokenType::AT: return "@";
        case TokenType::TILDE: return "~";
        case TokenType::AMPERSAND: return "&";
        case TokenType::PIPE: return "|";
        case TokenType::CARET: return "^";
        case TokenType::LESS_LESS: return "<<";
        case TokenType::GREATER_GREATER: return ">>";
        case TokenType::TILDE_EQUALS: return "~=";
        case TokenType::LEFT_PAREN: return "(";
        case TokenType::RIGHT_PAREN: return ")";
        case TokenType::LEFT_BRACKET: return "[";
        case TokenType::RIGHT_BRACKET: return "]";
        case TokenType::LEFT_BRACE: return "{";
        case TokenType::RIGHT_BRACE: return "}";
        case TokenType::BANG_EQUAL_OLD: return "bang equal old";
        case TokenType::EXCLAMATION: return "exclamation";
        case TokenType::RIGHT_ANGLE: return "right angle";
        case TokenType::EOF_TOKEN: return "eof";
        case TokenType::ERROR: return "error";
    }
    return "unknown";
}

static std::unordered_map<std::string, TokenType> keywords = []() {
    std::unordered_map<std::string, TokenType> map;
    map["type"] = TokenType::TYPE_KEYWORD;
    map["interface"] = TokenType::INTERFACE_KEYWORD;
    map["extends"] = TokenType::EXTENDS_KEYWORD;
    map["implements"] = TokenType::IMPLEMENTS_KEYWORD;
    map["enum"] = TokenType::ENUM_KEYWORD;
    map["const"] = TokenType::CONST_KEYWORD;
    map["let"] = TokenType::LET_KEYWORD;
    map["var"] = TokenType::VAR_KEYWORD;
    map["if"] = TokenType::IF_KEYWORD;
    map["else"] = TokenType::ELSE_KEYWORD;
    map["while"] = TokenType::WHILE_KEYWORD;
    map["for"] = TokenType::FOR_KEYWORD;
    map["do"] = TokenType::DO_KEYWORD;
    map["break"] = TokenType::BREAK_KEYWORD;
    map["continue"] = TokenType::CONTINUE_KEYWORD;
    map["return"] = TokenType::RETURN_KEYWORD;
    map["switch"] = TokenType::SWITCH_KEYWORD;
    map["case"] = TokenType::CASE_KEYWORD;
    map["default"] = TokenType::DEFAULT_KEYWORD;
    map["function"] = TokenType::FUNCTION_KEYWORD;
    map["class"] = TokenType::CLASS_KEYWORD;
    map["constructor"] = TokenType::CONSTRUCTOR_KEYWORD;
    map["new"] = TokenType::NEW_KEYWORD;
    map["super"] = TokenType::SUPER_KEYWORD;
    map["this"] = TokenType::THIS_KEYWORD;
    map["package"] = TokenType::PACKAGE_KEYWORD;
    map["export"] = TokenType::EXPORT_KEYWORD;
    map["import"] = TokenType::IMPORT_KEYWORD;
    map["from"] = TokenType::FROM_KEYWORD;
    map["module"] = TokenType::MODULE_KEYWORD;
    map["namespace"] = TokenType::NAMESPACE_KEYWORD;
      map["as"] = TokenType::AS_KEYWORD;
 map["range"] = TokenType::RANGE_KEYWORD;
    map["in"] = TokenType::IN_KEYWORD;
    map["public"] = TokenType::PUBLIC_KEYWORD;
    map["private"] = TokenType::PRIVATE_KEYWORD;
    map["protected"] = TokenType::PROTECTED_KEYWORD;
    map["static"] = TokenType::STATIC_KEYWORD;
    map["abstract"] = TokenType::ABSTRACT_KEYWORD;
    map["final"] = TokenType::FINAL_KEYWORD;
    map["any"] = TokenType::ANY_KEYWORD;
    map["unknown"] = TokenType::UNKNOWN_KEYWORD;
    map["never"] = TokenType::NEVER_KEYWORD;
    map["void"] = TokenType::VOID_KEYWORD;
    map["null"] = TokenType::NULL_KEYWORD;
    map["true"] = TokenType::TRUE_KEYWORD;
    map["false"] = TokenType::FALSE_KEYWORD;
map["undefined"] = TokenType::UNDEFINED_KEYWORD;
    map["struct"] = TokenType::STRUCT_KEYWORD;
    map["defer"] = TokenType::DEFER_KEYWORD;
    map["panic"] = TokenType::PANIC_KEYWORD;
    map["recover"] = TokenType::RECOVER_KEYWORD;
    map["go"] = TokenType::GO_KEYWORD;
    map["channel"] = TokenType::CHANNEL_KEYWORD;
    map["select"] = TokenType::SELECT_KEYWORD;
    map["mutex"] = TokenType::MUTEX_KEYWORD;
    map["waitgroup"] = TokenType::WAITGROUP_KEYWORD;
    map["try"] = TokenType::TRY_KEYWORD;
    map["catch"] = TokenType::CATCH_KEYWORD;
    map["throw"] = TokenType::THROW_KEYWORD;
    map["finally"] = TokenType::FINALLY_KEYWORD;
    map["func"] = TokenType::FUNCTION_KEYWORD;
    return map;
}();

TokenType KeywordTable::get(const std::string& identifier) {
    auto it = keywords.find(identifier);
    return it != keywords.end() ? it->second : TokenType::IDENTIFIER;
}

bool KeywordTable::isKeyword(const std::string& identifier) {
    return keywords.find(identifier) != keywords.end();
}

bool KeywordTable::isTypeKeyword(TokenType type) {
    return type == TokenType::TYPE_KEYWORD ||
           type == TokenType::INTERFACE_KEYWORD ||
           type == TokenType::ENUM_KEYWORD ||
           type == TokenType::CLASS_KEYWORD ||
           type == TokenType::ANY_KEYWORD ||
           type == TokenType::UNKNOWN_KEYWORD ||
           type == TokenType::NEVER_KEYWORD ||
           type == TokenType::VOID_KEYWORD;
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << Token::tokenTypeToString(token.type) << "(" << token.lexeme << ")";
    if (token.literal.isNumber()) {
        os << "=" << token.literal.numberValue;
    }
    return os;
}

const Token& TokenIterator::next() {
    if (index >= tokens.size()) {
        HS_PANIC("No more tokens");
    }
    return tokens[index++];
}

} // namespace hs
