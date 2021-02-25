
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((abs(testInputs.x) == vec4(1.25, 0.0, 0.75, 2.25).x && abs(testInputs.xy) == vec4(1.25, 0.0, 0.75, 2.25).xy) && abs(testInputs.xyz) == vec4(1.25, 0.0, 0.75, 2.25).xyz) && abs(testInputs) == vec4(1.25, 0.0, 0.75, 2.25) ? colorGreen : colorRed;
}
