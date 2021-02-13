
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool out_params_are_distinct(out float x, out float y) {
    x = 1.0;
    y = 2.0;
    return x == 1.0 && y == 2.0;
}
vec4 main() {
    float x = 0.0;
    return out_params_are_distinct(x, x) ? colorGreen : colorRed;
}
