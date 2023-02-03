
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    int zero = int(colorGreen.x);
    int one = int(colorGreen.y);
    for (int x = zero;x < 100; ++x) {
        for (int y = one;y < 100; ++y) {
            int _0_x = x;
            int _1_result = 0;
            while (_0_x >= y) {
                ++_1_result;
                _0_x -= y;
            }
            if (x / y != _1_result) {
                return vec4(1.0, float(x) * 0.003921569, float(y) * 0.003921569, 1.0);
            }
        }
    }
    return colorGreen;
}
