
out vec4 sk_FragColor;
in vec2 a;
in vec2 b;
float cross(vec2 a, vec2 b) {
    return a.x * b.y - a.y * b.x;
}
void main() {
    sk_FragColor.x = cross(a, b);
}
