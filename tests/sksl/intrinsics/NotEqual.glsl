
out vec4 sk_FragColor;
uniform vec4 a;
uniform vec4 b;
void main() {
    sk_FragColor.x = float(notEqual(a, b).x ? 1 : 0);
}
