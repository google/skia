
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(-2.0, 0.0, 0.0, 2.0);
    return ((floor(testInputs.x) == expected.x && floor(testInputs.xy) == expected.xy) && floor(testInputs.xyz) == expected.xyz) && floor(testInputs) == expected ? colorGreen : colorRed;
}
