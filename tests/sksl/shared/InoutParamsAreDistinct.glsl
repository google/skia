
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool inout_params_are_distinct_bhh(inout float x, inout float y) {
    x = 1.0;
    y = 2.0;
    return x == 1.0 && y == 2.0;
}
vec4 main() {
    float x = 0.0;
    return inout_params_are_distinct_bhh(x, x) ? colorGreen : colorRed;
}
