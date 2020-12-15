
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 c = color;
    if (c.x >= 0.5) {
        vec4 _0_ifBody;
        _0_ifBody = color + vec4(0.125);

        c = _0_ifBody;
    }
    sk_FragColor = c;
}
