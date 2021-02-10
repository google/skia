
out vec4 sk_FragColor;
vec4 helper();
struct Color {
    float red;
    float green;
    float blue;
    float alpha;
};
void main() {
    sk_FragColor = helper();
}
vec4 helper() {
    Color c;
    c.red = 0.25;
    c.green = 0.5;
    c.blue = 0.75;
    c.alpha = 1.0;
    return vec4(c.red, c.green, c.blue, c.alpha);
}
