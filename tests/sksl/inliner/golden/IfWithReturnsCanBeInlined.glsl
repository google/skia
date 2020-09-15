
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 color;
void main() {
    mediump vec4 _0_branchy;
    {
        if (color.z == color.w) _0_branchy = color.yyyy; else _0_branchy = color.zzzz;
    }

    sk_FragColor = _0_branchy;

}
