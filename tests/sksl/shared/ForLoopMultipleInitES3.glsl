
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float sumA = 0.0;
    float sumB = 0.0;
    {
        float a = 0.0;
        float b = 10.0;
        for (; a < 10.0 && b > 0.0; (++a, --b)) {
            sumA += a;
            sumB += b;
        }
    }
    if (sumA != 45.0 || sumB != 55.0) {
        return colorRed;
    }
    int sumC = 0;
    {
        int c = 0;
        for (; c < 10; ++c) {
            sumC += c;
        }
    }
    if (sumC != 45) {
        return colorRed;
    }
    float sumE = 0.0;
    {
        float d[2] = float[2](0.0, 10.0);
        float e[4] = float[4](1.0, 2.0, 3.0, 4.0);
        for (; d[0] < d[1]; ++d[0]) {
            sumE += e[0];
        }
    }
    if (sumE != 10.0) {
        return colorRed;
    }
    {
        for (; ; ) break;
    }
    for (; ; ) return colorGreen;
}
