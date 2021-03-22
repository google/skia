
out vec4 sk_FragColor;
uniform vec4 color;
float singleuse_h() {
    return 1.25;
}
float add_hhh(float a, float b) {
    float c = a + b;
    return c;
}
float mul_hhh(float a, float b) {
    return a * b;
}
float fma_hhhh(float a, float b, float c) {
    return add_hhh(mul_hhh(a, b), c);
}
void main() {
    sk_FragColor = vec4(fma_hhhh(color.x, color.y, color.z));
    sk_FragColor *= singleuse_h();
}
