
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    bool _0_test;
    {
        _0_test = color.x <= 0.5;
    }

    sk_FragColor = _0_test ? vec4(0.5) : vec4(1.0);

}
