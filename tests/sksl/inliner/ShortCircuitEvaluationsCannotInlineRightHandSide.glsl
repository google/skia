
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
    if (color.x <= 0.5 && testB(color)) {
        sk_FragColor = vec4(0.5);
    }
    if (color.x > 0.5 || testA(color)) {
        sk_FragColor = vec4(1.0);
    }
}
