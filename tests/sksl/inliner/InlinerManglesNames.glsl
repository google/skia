
out vec4 sk_FragColor;
uniform vec4 color;
vec4 main() {
    float _1_fma;
    float _2_mul;

    float _8_add;
    float _9_a = color.x * color.y;
    float _10_c = _9_a + color.z;
    float a = _10_c;


    float _3_fma;
    float _4_mul;

    float _11_add;
    float _12_a = color.y * color.z;
    float _13_c = _12_a + color.w;
    float b = _13_c;


    float _5_fma;
    float _6_mul;

    float _14_add;
    float _15_a = color.z * color.w;
    float _16_c = _15_a + color.x;
    float c = _16_c;


    float _7_mul;
    float _17_mul;
    float _18_mul;
    float _19_b = b * c;
    return vec4(a, b, c * c, a * _19_b);



}
