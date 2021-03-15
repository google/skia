
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_result;
    switch (int(color.x)) {
        case 1:
            _0_result = color.yyyy;
            break;
        default:
            _0_result = color.zzzz;
            break;
    }
    sk_FragColor = _0_result;
}
