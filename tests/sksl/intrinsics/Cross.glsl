
out vec4 sk_FragColor;
uniform vec2 a;
uniform vec2 b;
void main() {
    sk_FragColor.x = a.x * b.y - a.y * b.x;
}
