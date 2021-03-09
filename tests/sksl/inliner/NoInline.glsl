
out vec4 sk_FragColor;
uniform vec4 color;
float singleuse() {
    return 1.25;
}
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
void main() {
    sk_FragColor = vec4(fma(color.x, color.y, color.z));
    sk_FragColor *= singleuse();
}
