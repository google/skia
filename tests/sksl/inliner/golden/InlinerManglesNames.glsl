
uniform vec4 color;
vec4 main() {
    float _3_fma;
    float _4_a = color.x;
    float _5_b = color.y;
    float _6_c = color.z;
    {
        float _7_0_mul;
        {
            _7_0_mul = _4_a * _5_b;
        }

        float _8_1_add;
        {
            float _9_2_c = _7_0_mul + _6_c;
            _8_1_add = _9_2_c;
        }

        _3_fma = _8_1_add;

    }

    float a = _3_fma;

    float _10_fma;
    float _11_a = color.y;
    float _12_b = color.z;
    float _13_c = color.w;
    {
        float _14_0_mul;
        {
            _14_0_mul = _11_a * _12_b;
        }

        float _15_1_add;
        {
            float _16_2_c = _14_0_mul + _13_c;
            _15_1_add = _16_2_c;
        }

        _10_fma = _15_1_add;

    }

    float b = _10_fma;

    float _17_fma;
    float _18_a = color.z;
    float _19_b = color.w;
    float _20_c = color.x;
    {
        float _21_0_mul;
        {
            _21_0_mul = _18_a * _19_b;
        }

        float _22_1_add;
        {
            float _23_2_c = _21_0_mul + _20_c;
            _22_1_add = _23_2_c;
        }

        _17_fma = _22_1_add;

    }

    float c = _17_fma;

    float _24_mul;
    {
        _24_mul = c * c;
    }

    float _25_mul;
    {
        _25_mul = b * c;
    }

    float _26_mul;
    {
        _26_mul = a * _25_mul;
    }

    return vec4(a, b, _24_mul, _26_mul);

}
