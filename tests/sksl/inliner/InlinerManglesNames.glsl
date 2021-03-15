
out vec4 sk_FragColor;
uniform vec4 color;
float add(float a, float b) {
    float c = a + b;
    return c;
}
float mul(float a, float b) {
    return a * b;
}
vec4 main() {
    float a = add(color.x * color.y, color.z);
    float b = add(color.y * color.z, color.w);
    float c = add(color.z * color.w, color.x);
    return vec4(a, b, c * c, mul(a, b * c));
}
