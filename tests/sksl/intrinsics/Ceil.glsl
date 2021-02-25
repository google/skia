
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((ceil(testInputs.x) == vec4(-1.0, 0.0, 1.0, 3.0).x && ceil(testInputs.xy) == vec4(-1.0, 0.0, 1.0, 3.0).xy) && ceil(testInputs.xyz) == vec4(-1.0, 0.0, 1.0, 3.0).xyz) && ceil(testInputs) == vec4(-1.0, 0.0, 1.0, 3.0) ? colorGreen : colorRed;
}
