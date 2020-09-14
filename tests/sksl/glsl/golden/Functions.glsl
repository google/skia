
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    highp float x = 10.0;
    {
        highp float y[2], z;
        y[0] = 10.0;
        y[1] = 20.0;
        highp float _0_foo;
        {
            _0_foo = y[0] * y[1];
        }

        z = _0_foo;

        x = z;
    }


    sk_FragColor = vec4(x);
}
