
uniform vec4 color;
vec4 main() {
    float _1_fma;
    float _2_a = color.x;
    float _3_b = color.y;
    float _4_c = color.z;
    {
        float _5_0_mul;
        {
            _5_0_mul = _2_a * _3_b;
        }
        float _17_add;
        {
            float _18_c = _5_0_mul + _4_c;
            _17_add = _18_c;
        }
        _1_fma = _17_add;


    }
    float a = _1_fma;

    float _6_fma;
    float _7_a = color.y;
    float _8_b = color.z;
    float _9_c = color.w;
    {
        float _10_0_mul;
        {
            _10_0_mul = _7_a * _8_b;
        }
        float _19_add;
        {
            float _20_c = _10_0_mul + _9_c;
            _19_add = _20_c;
        }
        _6_fma = _19_add;


    }
    float b = _6_fma;

    float _11_fma;
    float _12_a = color.z;
    float _13_b = color.w;
    float _14_c = color.x;
    {
        float _15_0_mul;
        {
            _15_0_mul = _12_a * _13_b;
        }
        float _21_add;
        {
            float _22_c = _15_0_mul + _14_c;
            _21_add = _22_c;
        }
        _11_fma = _21_add;


    }
    float c = _11_fma;

    float _16_mul;
    {
        _16_mul = c * c;
    }
    float _23_mul;
    {
        _23_mul = b * c;
    }
    float _24_mul;
    {
        _24_mul = a * _23_mul;
    }
    return vec4(a, b, _16_mul, _24_mul);



}
