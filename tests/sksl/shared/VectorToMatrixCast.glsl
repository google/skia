
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 testInputs;
vec4 main() {
    return mat2(colorRed) == mat2(1.0) && mat2(testInputs) == mat2(-1.25, 0.0, 0.75, 2.25) ? colorGreen : colorRed;
}
