
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    a = 1;
    b = 2;
    c = 5;
    return ((a == 1 && b == 2) && c == 5) && d == 0 ? colorGreen : colorRed;
}
