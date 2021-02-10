
out vec4 sk_FragColor;
uniform vec4 color;
float add(float a, float b) {
    float c = a + b;
    return c;
}
float mul(float a, float b) {
    return a * b;
}
float fma(float a, float b, float c) {
    return add(mul(a, b), c);
}
vec4 main() {
    float a = fma(color.x, color.y, color.z);
    float b = fma(color.y, color.z, color.w);
    float c = fma(color.z, color.w, color.x);
    return vec4(a, b, mul(c, c), mul(a, mul(b, c)));
}
