
out vec4 sk_FragColor;
uniform vec4 color;
vec4 main() {
    float _2_a = color.x * color.y;
    float _3_c = _2_a + color.z;
    float a = _3_c;
    float _4_a = color.y * color.z;
    float _5_c = _4_a + color.w;
    float b = _5_c;
    float _6_a = color.z * color.w;
    float _7_c = _6_a + color.x;
    float c = _7_c;
    float _8_b = b * c;
    return vec4(a, b, c * c, a * _8_b);
}
