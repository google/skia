
uniform vec4 color;
vec4 main() {
    float _9_a = color.x * color.y;
    float _10_c = _9_a + color.z;
    float a = _10_c;


    float _12_a = color.y * color.z;
    float _13_c = _12_a + color.w;
    float b = _13_c;


    float _15_a = color.z * color.w;
    float _16_c = _15_a + color.x;
    float c = _16_c;


    float _19_b = b * c;
    return vec4(a, b, c * c, a * _19_b);



}
