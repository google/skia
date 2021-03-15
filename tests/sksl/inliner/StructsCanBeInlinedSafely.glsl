
out vec4 sk_FragColor;
vec4 helper();
struct Color {
    float red;
    float green;
    float blue;
    float alpha;
};
void main() {
    Color _0_c;
    _0_c.red = 0.25;
    _0_c.green = 0.5;
    _0_c.blue = 0.75;
    _0_c.alpha = 1.0;
    sk_FragColor = vec4(_0_c.red, _0_c.green, _0_c.blue, _0_c.alpha);
}
