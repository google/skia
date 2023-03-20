
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float x = 1.0;
    float y = 1.0;
    x == y ? (x += 1.0) : (y += 1.0);
    x == y ? (x += 3.0) : (y += 3.0);
    x < y ? (x += 5.0) : (y += 5.0);
    y >= x ? (x += 9.0) : (y += 9.0);
    x != y ? (x += 1.0) : y;
    x == y ? (x += 2.0) : y;
    x != y ? x : (y += 3.0);
    x == y ? x : (y += 4.0);
    bool b = true;
    bool c = (b = false) ? false : b;
    return c ? colorRed : (x == 8.0 && y == 17.0 ? colorGreen : colorRed);
}
