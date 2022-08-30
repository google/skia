
out vec4 sk_FragColor;
struct Color {
    float red;
    float green;
    float blue;
    float alpha;
};
vec4 helper_h4() {
    Color c;
    c.red = 0.0;
    c.green = 1.0;
    c.blue = 0.0;
    c.alpha = 1.0;
    return vec4(c.red, c.green, c.blue, c.alpha);
}
vec4 main() {
    return helper_h4();
}
