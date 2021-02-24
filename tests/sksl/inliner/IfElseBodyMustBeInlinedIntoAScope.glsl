
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 c = color;
    if (c.x >= 0.5) {
    } else {
        vec4 _0_elseBody;
        c = color + vec4(0.125);
    }
    sk_FragColor = c;
}
