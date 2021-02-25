
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    bool _0_test;
    sk_FragColor = color.x <= 0.5 ? vec4(0.5) : vec4(1.0);

}
