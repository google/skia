
out vec4 sk_FragColor;
uniform vec4 inColor;
vec4 flip(vec4 v) {
    return v.wzyx;
}
void mutating_flip(out vec4 v) {
    v = v.wzyx;
}
void main() {
    vec4 color = inColor;
    sk_FragColor = color.xyzy.wzyx;
    sk_FragColor = flip(color.xyzy);
    mutating_flip(color);
    sk_FragColor = color;
}
