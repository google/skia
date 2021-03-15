
out vec4 sk_FragColor;
uniform vec4 inColor;
void mutating_flip(out vec4 v) {
    v = v.wzyx;
}
void main() {
    vec4 color = inColor;
    sk_FragColor = color.yzyx;
    sk_FragColor = color.yzyx;
    mutating_flip(color);
    sk_FragColor = color;
}
