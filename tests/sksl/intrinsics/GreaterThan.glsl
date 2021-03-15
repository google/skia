
out vec4 sk_FragColor;
uniform vec4 a;
uniform vec4 b;
void main() {
    sk_FragColor.x = float(greaterThan(a, b).x ? 1 : 0);
}
