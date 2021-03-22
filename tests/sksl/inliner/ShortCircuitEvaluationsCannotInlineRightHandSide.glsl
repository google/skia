
out vec4 sk_FragColor;
uniform vec4 color;
bool testA_bh4(vec4 v) {
    return v.x <= 0.5;
}
bool testB_bh4(vec4 v) {
    return v.x > 0.5;
}
void main() {
    sk_FragColor = vec4(0.0);
    if (color.x <= 0.5 && testB_bh4(color)) {
        sk_FragColor = vec4(0.5);
    }
    if (color.x > 0.5 || testA_bh4(color)) {
        sk_FragColor = vec4(1.0);
    }
}
