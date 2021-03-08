
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float x = 0.0;
    x = 1.0;
    x = 2.0;
    return x == 1.0 && x == 2.0 ? colorGreen : colorRed;

}
