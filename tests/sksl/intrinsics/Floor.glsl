
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((floor(testInputs.x) == vec4(-2.0, 0.0, 0.0, 2.0).x && floor(testInputs.xy) == vec4(-2.0, 0.0, 0.0, 2.0).xy) && floor(testInputs.xyz) == vec4(-2.0, 0.0, 0.0, 2.0).xyz) && floor(testInputs) == vec4(-2.0, 0.0, 0.0, 2.0) ? colorGreen : colorRed;
}
