
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform vec4 colorWhite;
uniform vec4 colorBlack;
void setToColorBlack(out vec4 x) {
    x = colorBlack;
}
vec4 main() {
    vec4 a;
    vec4 b;
    vec4 c;
    vec4 d;
    (b = colorRed , c = colorGreen);
    a = (setToColorBlack(d) , colorWhite);
    a *= a;
    b *= b;
    c *= c;
    d *= d;
    return ((a == colorWhite && b == colorRed) && c == colorGreen) && d == colorBlack ? colorGreen : colorRed;
}
