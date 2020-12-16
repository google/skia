
uniform vec4 color;
vec4 main() {
    float _1_fma;
    float _2_mul;
    _2_mul = color.x * color.y;

    float _8_add;
    float _9_c = _2_mul + color.z;
    _8_add = _9_c;

    _1_fma = _8_add;



    float a = _1_fma;

    float _3_fma;
    float _4_mul;
    _4_mul = color.y * color.z;

    float _10_add;
    float _11_c = _4_mul + color.w;
    _10_add = _11_c;

    _3_fma = _10_add;



    float b = _3_fma;

    float _5_fma;
    float _6_mul;
    _6_mul = color.z * color.w;

    float _12_add;
    float _13_c = _6_mul + color.x;
    _12_add = _13_c;

    _5_fma = _12_add;



    float c = _5_fma;

    float _7_mul;
    _7_mul = c * c;

    float _14_mul;
    _14_mul = b * c;

    float _15_mul;
    _15_mul = a * _14_mul;

    return vec4(a, b, _7_mul, _15_mul);



}
