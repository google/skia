
out vec4 sk_FragColor;
uniform int value;
void main() {
    vec4 _0_loopy;
    for (int _1_x = 0;_1_x < 5; ++_1_x) {
        if (_1_x == value) _0_loopy = vec4(0.5);
    }
    _0_loopy = vec4(1.0);
    sk_FragColor = _0_loopy;
}
