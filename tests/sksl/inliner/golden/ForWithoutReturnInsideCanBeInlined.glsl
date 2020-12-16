
out vec4 sk_FragColor;
uniform int value;
void main() {
    vec4 _0_loopy;
    vec4 _1_result = vec4(1.0);
    for (int _2_x = 0;_2_x < 5; ++_2_x) {
        if (_2_x == value) _1_result = vec4(0.5);
    }
    _0_loopy = _1_result;

    sk_FragColor = _0_loopy;

}
