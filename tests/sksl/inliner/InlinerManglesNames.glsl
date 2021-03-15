
out vec4 sk_FragColor;
uniform vec4 color;
vec4 main() {
    float _0_a = color.x * color.y;
    float _1_c = _0_a + color.z;
    float a = _1_c;
    float _2_a = color.y * color.z;
    float _3_c = _2_a + color.w;
    float b = _3_c;
    float _4_a = color.z * color.w;
    float _5_c = _4_a + color.x;
    float c = _5_c;
    float _6_b = b * c;
    return vec4(a, b, c * c, a * _6_b);
}
