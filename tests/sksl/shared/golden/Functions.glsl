
out vec4 sk_FragColor;
void main() {
    float x = 10.0;
    {
        float _2_y[2];
        float _3_z;

        _2_y[0] = 10.0;
        _2_y[1] = 20.0;
        float _4_0_foo;
        {
            _4_0_foo = _2_y[0] * _2_y[1];
        }

        _3_z = _4_0_foo;

        float _5_a[2][3];
        _5_a[0][0] = 123.0;
        _5_a[1][2] = 456.0;
        float _6_1_arr;
        {
            _6_1_arr = _5_a[0][0] * _5_a[1][2];
        }

        x = _3_z + _6_1_arr;

    }

    sk_FragColor = vec4(x);
}
