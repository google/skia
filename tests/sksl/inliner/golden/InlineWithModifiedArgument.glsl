
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    mediump float _0_parameterWrite;
    mediump float _1_x = 1.0;
    {
        _1_x *= 2.0;
        _0_parameterWrite = _1_x;
    }

    sk_FragColor.x = _0_parameterWrite;

}
