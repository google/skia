
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    vec4 _0_result = colorGreen;
    for (int _1_x = 0;_1_x < 4; ++_1_x) {
        _0_result *= colorGreen;
    }
    return _0_result;
}
