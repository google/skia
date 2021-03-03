
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(-1.0, 0.0, 1.0, 3.0);
    return ((ceil(testInputs.x) == expected.x && ceil(testInputs.xy) == expected.xy) && ceil(testInputs.xyz) == expected.xyz) && ceil(testInputs) == expected ? colorGreen : colorRed;
}
