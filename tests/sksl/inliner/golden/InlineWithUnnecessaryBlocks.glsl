
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 color;
void main() {
    mediump vec4 _0_blocky;
    {
        {
            _0_blocky = color;
        }
    }

    sk_FragColor = _0_blocky;

}
