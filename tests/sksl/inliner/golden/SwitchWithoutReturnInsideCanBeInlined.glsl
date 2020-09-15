
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp int value;
void main() {
    mediump vec4 _0_switchy;
    {
        mediump vec4 _1_result = vec4(1.0);
        switch (value) {
            case 0:
                _1_result = vec4(0.5);
        }
        _0_switchy = _1_result;
    }

    sk_FragColor = _0_switchy;

}
