function alpha(value, color) {
    return value << 24 | (color & 0x00FFFFFF);
}

function argb(a, r, g, b) {
    return a << 24 | r << 16 | g << 8 | b;
}

function assert(condition) {
    if (!condition) debugger;
}

function isAlpha(code) {
    return (code > 64 && code < 91) // upper alpha (A-Z)
        || (code > 96 && code < 123); // lower alpha (a-z)
}

function isArray(a) {
    return a.constructor === Array;
}

function rgb(r, g, b) {
    return 0xFF << 24 | r << 16 | g << 8 | b;
}
