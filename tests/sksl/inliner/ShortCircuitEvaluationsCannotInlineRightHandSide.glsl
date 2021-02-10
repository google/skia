
out vec4 sk_FragColor;
uniform vec4 color;
bool testA(vec4 v) {
    return v.x <= 0.5;
}
bool testB(vec4 v) {
    return v.x > 0.5;
}
void main() {
    sk_FragColor = vec4(0.0);
    if (testA(color) && testB(color)) {
        sk_FragColor = vec4(0.5);
    }
    if (testB(color) || testA(color)) {
        sk_FragColor = vec4(1.0);
    }
}
