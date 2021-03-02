
out vec4 sk_FragColor;
uniform vec4 inColor;
void main() {
    vec4 color = inColor;
    sk_FragColor = color.xyzy.wzyx;
    sk_FragColor = color.xyzy.wzyx;

    color = color.wzyx;
    false;

    sk_FragColor = color;
}
