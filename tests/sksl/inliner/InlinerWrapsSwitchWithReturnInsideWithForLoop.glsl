
out vec4 sk_FragColor;
uniform int value;
void main() {
    vec4 _0_switchy;
    switch (value) {
        case 0:
            _0_switchy = vec4(0.5);
    }
    _0_switchy = vec4(1.0);
    sk_FragColor = _0_switchy;
}
