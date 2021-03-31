
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    const int _0_x[3] = int[3](1, 2, 3);
    const int _2_y[3] = int[3](1, 2, 4);
    return _0_x != _2_y && !(_0_x == _2_y) ? colorGreen : colorRed;
}
