
out vec4 sk_FragColor;
uniform vec4 colorGreen;
struct S {
    float x;
    int y;
};
struct Nested {
    S a;
};
vec4 main() {
    return colorGreen;
}
