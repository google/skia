
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
    bool _0_testA;
    _0_testA = color.x <= 0.5;

    if (_0_testA && testB(color)) {
        sk_FragColor = vec4(0.5);
    }

    bool _1_testB;
    _1_testB = color.x > 0.5;

    if (_1_testB || testA(color)) {
        sk_FragColor = vec4(1.0);
    }

}
