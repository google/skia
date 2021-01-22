
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_switchy;
    {
        vec4 _1_result;
        switch (int(color.x)) {
            case 1:
                _1_result = color.yyyy;
                break;
            default:
                _1_result = color.zzzz;
                break;
        }
        _0_switchy = _1_result;
    }

    sk_FragColor = _0_switchy;

}
