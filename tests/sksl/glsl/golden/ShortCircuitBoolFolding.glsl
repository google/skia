
out vec4 sk_FragColor;
void main() {
    bool expr1 = gl_FragCoord.x > 0.0;
    bool expr2 = gl_FragCoord.y > 0.0;
    if (expr1) {
        sk_FragColor.x = 1.0;
    } else if (!expr1) {
        sk_FragColor.x = 3.0;
    } else if (expr2) {
        sk_FragColor.x = 4.0;
    } else if (expr2) {
        sk_FragColor.x = 5.0;
    } else {
        sk_FragColor.x = 6.0;
    }
    if (expr1) {
        sk_FragColor.x = 1.0;
    } else if (!expr1) {
        sk_FragColor.x = 3.0;
    } else if (expr2) {
        sk_FragColor.x = 4.0;
    } else if (expr2) {
        sk_FragColor.x = 5.0;
    } else {
        sk_FragColor.x = 6.0;
    }
}
