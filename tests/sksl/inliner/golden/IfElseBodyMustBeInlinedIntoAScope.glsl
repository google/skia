
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 c = color;
    if (c.x >= 0.5) {
    } else {
        vec4 _0_elseBody;
        _0_elseBody = color + vec4(0.125);

        c = _0_elseBody;
    }
    sk_FragColor = c;
}
