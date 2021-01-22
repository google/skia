
out vec4 sk_FragColor;
in vec2 a;
in vec2 b;
void main() {
    sk_FragColor.x = a.x * b.y - a.y * b.x;

}
