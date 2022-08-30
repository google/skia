
out vec4 sk_FragColor;
vec4 colorGreen;
vec2 vector_f2f2f2(vec2 x, vec2 y) {
    x = normalize(y);
    return x;
}
vec4 main() {
    vector_f2f2f2(vec2(2.0), vec2(4.0));
    return colorGreen;
}
