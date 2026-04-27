// Strings Standard Library

export function concat(a, b) {
    return a + b;
}

export function length(s) {
    var count = 0;
    while (s[count] != 0) {
        count = count + 1;
    }
    return count;
}

export function substring(s, start, end) {
    var result = "";
    for (var i = start; i < end; i++) {
        result = result + s[i];
    }
    return result;
}

export function toUpper(s) {
    var result = "";
    for (var i = 0; i < length(s); i++) {
        var ch = s[i];
        if (ch >= "a" && ch <= "z") {
            result = result + (ch - 32);
        } else {
            result = result + ch;
        }
    }
    return result;
}

export function toLower(s) {
    var result = "";
    for (var i = 0; i < length(s); i++) {
        var ch = s[i];
        if (ch >= "A" && ch <= "Z") {
            result = result + (ch + 32);
        } else {
            result = result + ch;
        }
    }
    return result;
}

export function contains(s, substr) {
    var slen = length(s);
    var sublen = length(substr);
    if (sublen > slen) return false;
    
    for (var i = 0; i <= slen - sublen; i++) {
        var match = true;
        for (var j = 0; j < sublen; j++) {
            if (s[i + j] != substr[j]) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    return false;
}

export function startsWith(s, prefix) {
    var prefixLen = length(prefix);
    if (length(s) < prefixLen) return false;
    
    for (var i = 0; i < prefixLen; i++) {
        if (s[i] != prefix[i]) return false;
    }
    return true;
}

export function endsWith(s, suffix) {
    var suffixLen = length(suffix);
    var sLen = length(s);
    if (suffixLen > sLen) return false;
    
    for (var i = 0; i < suffixLen; i++) {
        if (s[sLen - suffixLen + i] != suffix[i]) return false;
    }
    return true;
}
