
out vec4 sk_FragColor;
vec4 main() {
    vec4 result = vec4(0.0);
    {
        float a = 0.0;
        float b = 0.0;
        for (; a < 10.0 && b < 10.0; (++a, ++b)) {
            result.x += a;
            result.y += b;
        }
    }
    {
        int c = 0;
        for (; c < 10; ++c) {
            result.z += 1.0;
        }
    }
    {
        float d[2] = float[2](0.0, 10.0);
        float e[4] = float[4](1.0, 2.0, 3.0, 4.0);
        float f = 9.0;
        for (; d[0] < d[1]; ++d[0]) {
            result.w = e[0] * f;
        }
    }
    {
        for (; ; ) break;
    }
    for (; ; ) break;
    return result;
}
