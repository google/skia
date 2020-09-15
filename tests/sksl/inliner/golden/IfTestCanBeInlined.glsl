
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 color;
void main() {
    bool _0_ifTest;
    {
        _0_ifTest = color.x >= 0.5;
    }

    if (_0_ifTest) sk_FragColor = vec4(1.0); else sk_FragColor = vec4(0.5);

}
