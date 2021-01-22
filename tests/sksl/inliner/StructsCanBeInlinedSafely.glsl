
out vec4 sk_FragColor;
vec4 helper();
struct Color {
    float red;
    float green;
    float blue;
    float alpha;
};
void main() {
    Color _1_c;
    _1_c.red = 0.25;
    _1_c.green = 0.5;
    _1_c.blue = 0.75;
    _1_c.alpha = 1.0;
    sk_FragColor = vec4(_1_c.red, _1_c.green, _1_c.blue, _1_c.alpha);

}
