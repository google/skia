
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float _0_get;
    for (int _1_loop = 0;_1_loop < 1; _1_loop++) {
        {
            {
                _0_get = abs(2.0);
                continue;
            }
        }
        {
            _0_get = abs(5.0);
            continue;
        }
    }
    float result = _0_get;
    return result == 2.0 ? colorGreen : colorRed;
}
