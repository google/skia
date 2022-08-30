
out vec4 sk_FragColor;
uniform vec4 colorGreen;
int exact_division_iii(int x, int y) {
    int result = 0;
    while (x >= y) {
        ++result;
        x -= y;
    }
    return result;
}
vec4 main() {
    int zero = int(colorGreen.x);
    int one = int(colorGreen.y);
    for (int x = zero;x < 100; ++x) {
        for (int y = one;y < 100; ++y) {
            if (x / y != exact_division_iii(x, y)) {
                return vec4(1.0, float(x) / 255.0, float(y) / 255.0, 1.0);
            }
        }
    }
    return colorGreen;
}
