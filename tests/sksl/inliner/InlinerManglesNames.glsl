
out vec4 sk_FragColor;
uniform vec4 color;
float add(float a, float b) {
    float c = a + b;
    return c;
}
vec4 main() {
    float a = add(color.x * color.y, color.z);
    float b = add(color.y * color.z, color.w);
    float c = add(color.z * color.w, color.x);
    float _0_b = b * c;
    return vec4(a, b, c * c, a * _0_b);
}
