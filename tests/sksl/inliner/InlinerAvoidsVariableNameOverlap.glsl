
out vec4 sk_FragColor;
in vec2 x;
vec2 InlineB(vec2 tmp) {
    vec2 reusedName = tmp + vec2(3.0, 4.0);
    return reusedName;
}
vec2 InlineA() {
    vec2 reusedName = x + vec2(1.0, 2.0);
    return InlineB(reusedName);
}
vec4 main() {
    return InlineA().xyxy;
}
