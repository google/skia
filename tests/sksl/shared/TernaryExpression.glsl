
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    int check = 0;
    check += colorGreen.y == 1.0 ? 0 : 1;
    check += int(colorGreen.x == 1.0);
    check += colorGreen.yx == colorRed.xy ? 0 : 1;
    check += int(colorGreen.yx != colorRed.xy);
    return check == 0 ? colorGreen : colorRed;
}
