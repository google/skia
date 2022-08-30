
out vec4 sk_FragColor;
vec2 InlineB_h2h2(vec2 tmp) {
    vec2 reusedName = tmp - 1.0;
    return reusedName;
}
vec2 InlineA_h2() {
    vec2 reusedName = vec2(1.0, 2.0);
    return InlineB_h2h2(reusedName);
}
vec4 main() {
    return vec4(InlineA_h2(), 0.0, 1.0);
}
