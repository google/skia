
out vec4 sk_FragColor;
uniform vec4 inColor;
void main() {
    vec4 color = inColor;
    sk_FragColor = color.yzyx;
    sk_FragColor = color.yzyx;
    sk_FragColor = color;
}
