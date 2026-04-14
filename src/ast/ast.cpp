#include "ast.h"

namespace hs {

std::string BinaryExpr::opToString(BinaryOp op) {
    switch (op) {
        case BinaryOp::EQUAL: return "=";
        case BinaryOp::PLUS: return "+";
        case BinaryOp::MINUS: return "-";
        case BinaryOp::STAR: return "*";
        case BinaryOp::SLASH: return "/";
        case BinaryOp::PERCENT: return "%";
        case BinaryOp::PLUS_EQUAL: return "+=";
        case BinaryOp::MINUS_EQUAL: return "-=";
        case BinaryOp::STAR_EQUAL: return "*=";
        case BinaryOp::SLASH_EQUAL: return "/=";
        case BinaryOp::PERCENT_EQUAL: return "%=";
        case BinaryOp::EQUAL_EQUAL: return "==";
        case BinaryOp::BANG_EQUAL: return "!=";
        case BinaryOp::LESS: return "<";
        case BinaryOp::LESS_EQUAL: return "<=";
        case BinaryOp::GREATER: return ">";
        case BinaryOp::GREATER_EQUAL: return ">=";
        case BinaryOp::AND: return "&&";
        case BinaryOp::OR: return "||";
        case BinaryOp::AMPERSAND: return "&";
        case BinaryOp::PIPE: return "|";
        case BinaryOp::CARET: return "^";
        case BinaryOp::TILDE: return "~";
        case BinaryOp::LESS_LESS: return "<<";
        case BinaryOp::GREATER_GREATER: return ">>";
        case BinaryOp::INSTANCE_OF: return "instanceof";
        case BinaryOp::IN: return "in";
        case BinaryOp::TYPEOF: return "typeof";
        default: return "?";
    }
}

std::string UnaryExpr::opToString(UnaryOp op) {
    switch (op) {
        case UnaryOp::MINUS: return "-";
        case UnaryOp::BANG: return "!";
        case UnaryOp::TILDE: return "~";
        case UnaryOp::PLUS_PLUS: return "++";
        case UnaryOp::MINUS_MINUS: return "--";
        case UnaryOp::TYPEOF: return "typeof";
        case UnaryOp::VOID_OP: return "void";
        case UnaryOp::DELETE_OP: return "delete";
        default: return "?";
    }
}

} // namespace hs
