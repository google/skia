
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(0.0);
    for (highp int x = 0;x < 4; ++x) {
        mediump vec4 _0_adjust;
        {
            _0_adjust = sk_FragColor + vec4(0.125);
        }

        sk_FragColor = _0_adjust;
    }
}
