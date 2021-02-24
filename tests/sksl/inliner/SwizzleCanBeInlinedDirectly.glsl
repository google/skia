
out vec4 sk_FragColor;
uniform vec4 inColor;
void main() {
    vec4 color = inColor;
    sk_FragColor = color.xyzy.wzyx;
    vec4 _0_flip;
    sk_FragColor = color.xyzy.wzyx;

    color = color.wzyx;
    false;

    sk_FragColor = color;
}
