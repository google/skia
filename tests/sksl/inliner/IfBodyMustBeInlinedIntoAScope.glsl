
out vec4 sk_FragColor;
uniform vec4 color;
vec4 ifBody() {
    return color + vec4(0.125);
}
void main() {
    vec4 c = color;
    if (c.x >= 0.5) c = ifBody();
    sk_FragColor = c;
}
