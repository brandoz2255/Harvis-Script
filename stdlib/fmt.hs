// Format Standard Library

export function print(...) {
    // Print without newline
    for (var i = 0; i < count(...); i++) {
        writeChar(...[i]);
    }
}

export function println(...) {
    // Print with newline
    for (var i = 0; i < count(...); i++) {
        writeChar(...[i]);
    }
    writeChar(10); // newline
}

export function formatInt(n) {
    if (n == 0) return "0";
    
    var result = "";
    var neg = false;
    if (n < 0) {
        neg = true;
        n = -n;
    }
    
    while (n > 0) {
        var digit = n % 10;
        result = (char)(digit + 48) + result;
        n = n / 10;
    }
    
    if (neg) result = "-" + result;
    return result;
}

export function formatFloat(f, decimals) {
    var result = "";
    var neg = false;
    if (f < 0) {
        neg = true;
        f = -f;
    }
    
    var intPart = (int)f;
    var fracPart = f - intPart;
    
    for (var i = 0; i < decimals; i++) {
        fracPart = fracPart * 10;
        var digit = (int)fracPart;
        fracPart = fracPart - digit;
        result = result + (char)(digit + 48);
    }
    
    if (neg) result = "-" + result;
    return result;
}

export function sprintf(format, ...) {
    var result = "";
    var i = 0;
    var argIdx = 0;
    
    while (i < length(format)) {
        if (format[i] == "%") {
            i = i + 1;
            if (i >= length(format)) break;
            
            var spec = format[i];
            if (spec == "s") {
                result = result + formatString(arg...[argIdx]);
            } else if (spec == "d") {
                result = result + formatInt(formatInt(...[argIdx]));
            } else if (spec == "f") {
                result = result + formatFloat(...[argIdx], 2);
            } else {
                result = result + spec;
            }
            argIdx = argIdx + 1;
        } else {
            result = result + format[i];
        }
        i = i + 1;
    }
    
    return result;
}
