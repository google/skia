
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 color;
void main() {
    mediump vec4 c = color;
    if (c.x >= 0.5) {
    } else {
        mediump vec4 _0_elseBody;
        {
            _0_elseBody = color + vec4(0.125);
        }

        c = _0_elseBody;
    }
    sk_FragColor = c;
}
