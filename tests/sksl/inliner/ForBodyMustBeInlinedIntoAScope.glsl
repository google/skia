
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 multiply_h4h4h(vec4 x, float y) {
    vec4 value = x * y;
    return value;
}
vec4 main() {
    vec4 result;
    for (int x = 0;x < 4; ++x) result = multiply_h4h4h(colorGreen, 1.0);
    return result;
}
