#ifndef HARVIS_CORE_SCANNER_H
#define HARVIS_CORE_SCANNER_H

#include "types.h"
#include <string>
#include <vector>
#include <deque>
#include <cctype>

namespace hs {

// Character buffer for efficient scanning
class CharBuffer {
public:
    CharBuffer(const std::string& src) : source(src), pos(0), curLine(1), curCol(1) {}
    
    bool hasNext() const { return pos < source.size(); }
    char peek() const { return hasNext() ? source[pos] : '\0'; }
    char peekNext() const { return pos + 1 < source.size() ? source[pos + 1] : '\0'; }
    char advance() { 
        char c = peek();
        if (c == '\n') { ++curLine; curCol = 1; }
        else { ++curCol; }
        ++pos;
        return c;
    }
    
    void skip() { advance(); }
    
    bool match(char expected) {
        if (!hasNext() || peek() != expected) return false;
        advance();
        return true;
    }
    
    bool match(char a, char b) {
        char c = peek();
        if (c == a || c == b) {
            advance();
            return true;
        }
        return false;
    }
    
    bool check(char expected) const { return hasNext() && peek() == expected; }
    bool checkAny(const std::string& chars) const {
        for (char c : chars) if (check(c)) return true;
        return false;
    }
    
    template<typename Predicate>
    std::string takeWhile(Predicate predicate) {
        std::string result;
        while (hasNext() && predicate(peek())) {
            result += advance();
        }
        return result;
    }
    
    std::string takeN(int n) {
        std::string result;
        for (int i = 0; i < n && hasNext(); ++i) {
            result += advance();
        }
        return result;
    }
    
    void rewind(int n) {
        if (pos >= static_cast<size_t>(n)) {
            pos -= n;
            curLine = 1; curCol = 1;
            for (size_t i = 0; i < pos; ++i) {
                if (source[i] == '\n') { ++curLine; curCol = 1; }
                else { ++curCol; }
            }
        }
    }
    
    SourceLocation location() const {
        SourceLocation loc;
        loc.line = curLine;
        loc.column = curCol;
        return loc;
    }
    
    int getLine() const { return curLine; }
    int getCol() const { return curCol; }
    size_t getPosition() const { return pos; }

private:
    const std::string& source;
    size_t pos;
    int curLine;
    int curCol;
};

// String interpolation scanner
class InterpolationScanner {
public:
    InterpolationScanner(const std::string& src, SourceLocation startLoc)
        : buffer(src), startPos(startLoc) {}
    
    std::string scanString() {
        std::string content;
        bool inTemplate = false;
        
        while (buffer.hasNext()) {
            char c = buffer.peek();
            
            if (inTemplate) {
                if (c == '}' && buffer.peekNext() != '}') {
                    buffer.advance();
                    inTemplate = false;
                    continue;
                }
                content += buffer.advance();
            } else {
                if (c == '\\' && buffer.peekNext() != '\0') {
                    buffer.advance();
                    char next = buffer.advance();
                    switch (next) {
                        case 'n': content += '\n'; break;
                        case 't': content += '\t'; break;
                        case 'r': content += '\r'; break;
                        case '\\': content += '\\'; break;
                        case '"': content += '"'; break;
                        case '\'': content += '\''; break;
                        default: content += next;
                    }
                } else if (c == '{' && buffer.peekNext() != '{') {
                    buffer.advance();
                    inTemplate = true;
                } else {
                    content += buffer.advance();
                }
            }
        }
        
        return content;
    }
    
private:
    CharBuffer buffer;
    SourceLocation startPos;
};

} // namespace hs

#endif // HARVIS_CORE_SCANNER_H
