
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    highp float x = 10.0;
    {
        highp float _1_y[2], _2_z;
        _1_y[0] = 10.0;
        _1_y[1] = 20.0;
        highp float _3_0_foo;
        {
            _3_0_foo = _1_y[0] * _1_y[1];
        }

        _2_z = _3_0_foo;

        x = _2_z;
    }


    sk_FragColor = vec4(x);
}
