// Math Standard Library

export function add(a, b) {
    return a + b;
}

export function subtract(a, b) {
    return a - b;
}

export function multiply(a, b) {
    return a * b;
}

export function divide(a, b) {
    if (b == 0) {
        throw "Division by zero";
    }
    return a / b;
}

export function mod(a, b) {
    return a % b;
}

export function pow(base, exp) {
    var result = 1;
    for (var i = 0; i < exp; i++) {
        result = result * base;
    }
    return result;
}

export function sqrt(x) {
    if (x < 0) {
        throw "Cannot compute square root of negative number";
    }
    if (x == 0) return 0;
    
    var guess = x / 2;
    var prev = 0;
    
    for (var i = 0; i < 100; i++) {
        prev = guess;
        guess = (guess + x / guess) / 2;
        if (abs(guess - prev) < 0.0000001) {
            break;
        }
    }
    
    return guess;
}

export function abs(x) {
    if (x < 0) return -x;
    return x;
}

export function min(a, b) {
    if (a < b) return a;
    return b;
}

export function max(a, b) {
    if (a > b) return a;
    return b;
}

export constant PI = 3.14159265358979323846;
export constant E = 2.71828182845904523536;
