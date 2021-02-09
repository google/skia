
out vec4 sk_FragColor;
void n() {
    -ivec2(2, 1) == -(-ivec2(2, 1));
}
void main() {
    n();
    sk_FragColor = vec4(0.0);
}
