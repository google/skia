
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp int value;
void main() {
    mediump vec4 _0_loopy;
    {
        mediump vec4 _1_result = vec4(1.0);
        for (highp int _2_x = 0;_2_x < 5; ++_2_x) {
            if (_2_x == value) _1_result = vec4(0.5);
        }
        _0_loopy = _1_result;
    }

    sk_FragColor = _0_loopy;

}
