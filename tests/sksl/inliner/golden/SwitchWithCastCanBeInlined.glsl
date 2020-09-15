
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 color;
void main() {
    mediump vec4 _0_switchy;
    {
        mediump vec4 _1_result;
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
