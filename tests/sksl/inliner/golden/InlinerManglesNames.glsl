
uniform vec4 color;
vec4 main() {
    float _3_fma;
    {
        float _4_0_mul;
        {
            _4_0_mul = color.x * color.y;
        }

        float _5_1_add;
        {
            float _6_2_c = _4_0_mul + color.z;
            _5_1_add = _6_2_c;
        }

        _3_fma = _5_1_add;

    }

    float a = _3_fma;

    float _7_fma;
    {
        float _8_0_mul;
        {
            _8_0_mul = color.y * color.z;
        }

        float _9_1_add;
        {
            float _10_2_c = _8_0_mul + color.w;
            _9_1_add = _10_2_c;
        }

        _7_fma = _9_1_add;

    }

    float b = _7_fma;

    float _11_fma;
    {
        float _12_0_mul;
        {
            _12_0_mul = color.z * color.w;
        }

        float _13_1_add;
        {
            float _14_2_c = _12_0_mul + color.x;
            _13_1_add = _14_2_c;
        }

        _11_fma = _13_1_add;

    }

    float c = _11_fma;

    float _15_mul;
    {
        _15_mul = c * c;
    }

    float _16_mul;
    {
        _16_mul = b * c;
    }

    float _17_mul;
    {
        _17_mul = a * _16_mul;
    }

    return vec4(a, b, _15_mul, _17_mul);

}
