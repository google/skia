
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((max(testInputs.x, 0.5) == vec4(0.5, 0.5, 0.75, 2.25).x && max(testInputs.xy, 0.5) == vec4(0.5, 0.5, 0.75, 2.25).xy) && max(testInputs.xyz, 0.5) == vec4(0.5, 0.5, 0.75, 2.25).xyz) && max(testInputs, 0.5) == vec4(0.5, 0.5, 0.75, 2.25)) && max(testInputs.x, colorGreen.x) == vec4(0.0, 1.0, 0.75, 2.25).x) && max(testInputs.xy, colorGreen.xy) == vec4(0.0, 1.0, 0.75, 2.25).xy) && max(testInputs.xyz, colorGreen.xyz) == vec4(0.0, 1.0, 0.75, 2.25).xyz) && max(testInputs, colorGreen) == vec4(0.0, 1.0, 0.75, 2.25) ? colorGreen : colorRed;
}
