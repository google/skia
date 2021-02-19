
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((sign(testInputs.x) == vec4(-1.0, 0.0, 1.0, 1.0).x && sign(testInputs.xy) == vec4(-1.0, 0.0, 1.0, 1.0).xy) && sign(testInputs.xyz) == vec4(-1.0, 0.0, 1.0, 1.0).xyz) && sign(testInputs) == vec4(-1.0, 0.0, 1.0, 1.0) ? colorGreen : colorRed;
}
