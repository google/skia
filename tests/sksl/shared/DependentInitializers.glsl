
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float x = 0.5;
    float y = x * 2.0;
    return y == 1.0 ? colorGreen : colorRed;
}
