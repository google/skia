
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 unpremul_h4h4(vec4 color);
vec4 unpremul_h4h4(vec4 color) {
    return vec4(color.xyz / max(color.w, 0.0001), color.w);
}
vec4 live_fn_h4h4h4(vec4 a, vec4 b) {
    return a + b;
}
vec4 main() {
    vec4 a;
    vec4 b;
    {
        a = live_fn_h4h4h4(vec4(3.0), vec4(-5.0));
    }
    {
        b = unpremul_h4h4(vec4(1.0));
    }
    return a != vec4(0.0) && b != vec4(0.0) ? colorGreen : colorRed;
}
