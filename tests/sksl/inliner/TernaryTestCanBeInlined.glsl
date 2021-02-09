
out vec4 sk_FragColor;
uniform vec4 color;
bool test(vec4 v) {
    return v.x <= 0.5;
}
void main() {
    sk_FragColor = test(color) ? vec4(0.5) : vec4(1.0);
}
