
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    int _0_x[3] = int[3](1, 2, 3);
    int _1_xx[3] = int[3](1, 2, 3);
    int _2_y[3] = int[3](1, 2, 4);
    return (_0_x == _1_xx && !(_0_x == _2_y)) && _1_xx != _2_y ? colorGreen : colorRed;
}
