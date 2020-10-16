
out vec4 sk_FragColor;
uniform vec2 h2;
uniform vec2 f2;
float cross(vec2 a, vec2 b) {
    return a.x * b.y - a.y * b.x;
}
float cross(vec2 a, vec2 b) {
    return a.x * b.y - a.y * b.x;
}
void main() {
    sk_FragColor = vec4(cross(vec2(1.0, 2.0), vec2(3.0, 4.0)));
    sk_FragColor = vec4(cross(vec2(5.0, 6.0), vec2(7.0, 8.0)));
    sk_FragColor = vec4(cross(h2, h2));
    sk_FragColor = vec4(cross(f2, f2));
}
