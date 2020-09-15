
precision mediump float;
precision mediump sampler2D;
uniform mediump vec4 color;
mediump vec4 main() {
    mediump float _3_fma;
    mediump float _4_a = color.x;
    mediump float _5_b = color.y;
    mediump float _6_c = color.z;
    {
        mediump float _7_0_mul;
        {
            _7_0_mul = _4_a * _5_b;
        }

        mediump float _8_1_add;
        {
            mediump float _9_2_c = _7_0_mul + _6_c;
            _8_1_add = _9_2_c;
        }

        _3_fma = _8_1_add;

    }

    mediump float a = _3_fma;

    mediump float _10_fma;
    mediump float _11_a = color.y;
    mediump float _12_b = color.z;
    mediump float _13_c = color.w;
    {
        mediump float _14_0_mul;
        {
            _14_0_mul = _11_a * _12_b;
        }

        mediump float _15_1_add;
        {
            mediump float _16_2_c = _14_0_mul + _13_c;
            _15_1_add = _16_2_c;
        }

        _10_fma = _15_1_add;

    }

    mediump float b = _10_fma;

    mediump float _17_fma;
    mediump float _18_a = color.z;
    mediump float _19_b = color.w;
    mediump float _20_c = color.x;
    {
        mediump float _21_0_mul;
        {
            _21_0_mul = _18_a * _19_b;
        }

        mediump float _22_1_add;
        {
            mediump float _23_2_c = _21_0_mul + _20_c;
            _22_1_add = _23_2_c;
        }

        _17_fma = _22_1_add;

    }

    mediump float c = _17_fma;

    mediump float _24_mul;
    {
        _24_mul = c * c;
    }

    mediump float _25_mul;
    {
        _25_mul = b * c;
    }

    mediump float _26_mul;
    {
        _26_mul = a * _25_mul;
    }

    return vec4(a, b, _24_mul, _26_mul);

}
