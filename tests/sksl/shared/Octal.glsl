
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    int i1 = 1;
    int i2 = 342391;
    int i3 = 2000000000;
    int i4 = -2000000000;
    return ((i1 == 1 && i2 == 342391) && i3 == 2000000000) && i4 == -2000000000 ? colorGreen : colorRed;
}
