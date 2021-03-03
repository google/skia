
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((min(testInputs.x, 0.5) == vec4(-1.25, 0.0, 0.5, 0.5).x && min(testInputs.xy, 0.5) == vec4(-1.25, 0.0, 0.5, 0.5).xy) && min(testInputs.xyz, 0.5) == vec4(-1.25, 0.0, 0.5, 0.5).xyz) && min(testInputs, 0.5) == vec4(-1.25, 0.0, 0.5, 0.5)) && min(testInputs.x, colorGreen.x) == vec4(-1.25, 0.0, 0.0, 1.0).x) && min(testInputs.xy, colorGreen.xy) == vec4(-1.25, 0.0, 0.0, 1.0).xy) && min(testInputs.xyz, colorGreen.xyz) == vec4(-1.25, 0.0, 0.0, 1.0).xyz) && min(testInputs, colorGreen) == vec4(-1.25, 0.0, 0.0, 1.0) ? colorGreen : colorRed;
}
