
out vec4 sk_FragColor;
uniform vec4 color;
vec4 main() {
    float _0_c = color.x * color.y + color.z;
    float a = _0_c;
    float _1_c = color.y * color.z + color.w;
    float b = _1_c;
    float _2_c = color.z * color.w + color.x;
    float c = _2_c;
    return vec4(a, b, c * c, a * (b * c));
}
