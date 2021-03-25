
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    vec4 _0_result;
    for (int _1_x = 0;_1_x < 4; ++_1_x) {
        _0_result[_1_x] = colorGreen[_1_x];
    }
    return _0_result;
}
