
out vec4 sk_FragColor;
uniform vec4 color;
float add_hhh(float a, float b) {
    float c = a + b;
    return c;
}
float mul_hhh(float a, float b) {
    return a * b;
}
float fused_multiply_add_hhhh(float a, float b, float c) {
    return add_hhh(mul_hhh(a, b), c);
}
vec4 main() {
    float a = fused_multiply_add_hhhh(color.x, color.y, color.z);
    float b = fused_multiply_add_hhhh(color.y, color.z, color.w);
    float c = fused_multiply_add_hhhh(color.z, color.w, color.x);
    return vec4(a, b, mul_hhh(c, c), mul_hhh(a, mul_hhh(b, c)));
}
