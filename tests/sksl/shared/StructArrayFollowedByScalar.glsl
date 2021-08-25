
out vec4 sk_FragColor;
struct S {
    float rgb[3];
    float a;
};
vec4 main() {
    S s;
    s.rgb[0] = 0.0;
    s.rgb[1] = 1.0;
    s.rgb[2] = 0.0;
    s.a = 1.0;
    return vec4(s.rgb[0], s.rgb[1], s.rgb[2], s.a);
}
