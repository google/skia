
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return floor(testInputs.x) == -2.0 ? colorGreen : colorRed;
}
