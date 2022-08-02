
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 multiplyByAlpha_h4h4(vec4 x) {
    return x * x.wwww;
}
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
    vec4 result = vec3(vec2(fused_multiply_add_hhhh(colorGreen.w, colorGreen.y, colorGreen.x)), 0.0).zxzy;
    result = multiplyByAlpha_h4h4(result);
    return result;
}
