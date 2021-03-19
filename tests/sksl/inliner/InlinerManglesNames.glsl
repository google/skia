
out vec4 sk_FragColor;
uniform vec4 color;
vec4 main() {
    float _1_c = color.x * color.y + color.z;
    float a = _1_c;
    float _2_c = color.y * color.z + color.w;
    float b = _2_c;
    float _3_c = color.z * color.w + color.x;
    float c = _3_c;
    return vec4(a, b, c * c, a * (b * c));
}
