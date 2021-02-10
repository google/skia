
out vec4 sk_FragColor;
uniform vec4 color;
bool ifTest(vec4 v) {
    return color.x >= 0.5;
}
void main() {
    if (ifTest(color)) sk_FragColor = vec4(1.0); else sk_FragColor = vec4(0.5);
}
