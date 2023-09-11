
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
float sumArrayMutating_ff(float a[5]) {
    for (int i = 1;i < 5; ++i) {
        a[0] += a[i];
    }
    return a[0];
}
vec4 main() {
    float data[5];
    data[0] = 1.0;
    data[1] = 2.0;
    data[2] = 3.0;
    data[3] = 4.0;
    data[4] = 5.0;
    float _0_sum = 0.0;
    for (int _1_i = 0;_1_i < 5; ++_1_i) {
        _0_sum += data[_1_i];
    }
    return _0_sum == sumArrayMutating_ff(data) ? colorGreen : colorRed;
}
