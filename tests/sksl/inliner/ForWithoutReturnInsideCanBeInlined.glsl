
out vec4 sk_FragColor;
uniform int value;
void main() {
    vec4 _0_result = vec4(1.0);
    for (int _1_x = 0;_1_x < 5; ++_1_x) {
        if (_1_x == value) _0_result = vec4(0.5);
    }
    sk_FragColor = _0_result;
}
