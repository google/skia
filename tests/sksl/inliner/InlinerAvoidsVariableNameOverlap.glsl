
out vec4 sk_FragColor;
in vec2 x;
vec4 main() {
    vec2 _1_reusedName = x + vec2(1.0, 2.0);
    vec2 _2_reusedName = _1_reusedName + vec2(3.0, 4.0);
    return _2_reusedName.xyxy;
}
