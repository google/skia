
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    sk_FragColor = color.x <= 0.5 ? vec4(0.5) : vec4(1.0);
}
