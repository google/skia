
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _1_result;
    switch (int(color.x)) {
        case 1:
            _1_result = color.yyyy;
            break;
        default:
            _1_result = color.zzzz;
            break;
    }
    sk_FragColor = _1_result;

}
