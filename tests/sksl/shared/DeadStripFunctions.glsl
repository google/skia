
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 unpremul(vec4 color) {
    return vec4(color.xyz / max(color.w, 9.9999997473787516e-05), color.w);
}
vec4 unpremul_float(vec4 color) {
    return vec4(color.xyz / max(color.w, 9.9999997473787516e-05), color.w);
}
vec4 dead_fn(vec4 a, vec4 b) {
    return a * b;
}
vec4 live_fn(vec4 a, vec4 b) {
    return a + b;
}
vec4 main() {
    bool TRUE = true;
    bool FALSE = false;

    vec4 a;
    vec4 b;

    if (FALSE) {
        vec4 unused = dead_fn(vec4(0.5), vec4(2.0));
    } else {
        a = live_fn(vec4(3.0), vec4(-5.0));
    }
    if (TRUE) {
        b = unpremul(vec4(1.0));
    } else {
        vec4 unused = unpremul_float(vec4(-1.0));
    }
    return a != vec4(0.0) && b != vec4(0.0) ? colorGreen : colorRed;
}
