
out vec4 sk_FragColor;
struct Color {
    float red;
    float green;
    float blue;
    float alpha;
};
vec4 main() {
    Color _0_c;
    _0_c.red = 0.0;
    _0_c.green = 1.0;
    _0_c.blue = 0.0;
    _0_c.alpha = 1.0;
    return vec4(_0_c.red, _0_c.green, _0_c.blue, _0_c.alpha);
}
