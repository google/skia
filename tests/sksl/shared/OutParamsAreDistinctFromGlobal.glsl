
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
float x = 1.0;
bool out_params_are_distinct_from_global_bh(out float y) {
    y = 2.0;
    return x == 1.0 && y == 2.0;
}
vec4 main() {
    return out_params_are_distinct_from_global_bh(x) ? colorGreen : colorRed;
}
