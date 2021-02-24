
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    bool _0_ifTest;
    if (color.x >= 0.5) sk_FragColor = vec4(1.0); else sk_FragColor = vec4(0.5);

}
