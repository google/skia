
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((clamp(testInputs.x, -1.0, 1.0) == vec4(-1.0, 0.0, 0.75, 1.0).x && clamp(testInputs.xy, -1.0, 1.0) == vec4(-1.0, 0.0, 0.75, 1.0).xy) && clamp(testInputs.xyz, -1.0, 1.0) == vec4(-1.0, 0.0, 0.75, 1.0).xyz) && clamp(testInputs, -1.0, 1.0) == vec4(-1.0, 0.0, 0.75, 1.0)) && clamp(testInputs.x, vec4(-1.0, -2.0, -2.0, 1.0).x, vec4(1.0, 2.0, 0.5, 3.0).x) == vec4(-1.0, 0.0, 0.5, 2.25).x) && clamp(testInputs.xy, vec4(-1.0, -2.0, -2.0, 1.0).xy, vec4(1.0, 2.0, 0.5, 3.0).xy) == vec4(-1.0, 0.0, 0.5, 2.25).xy) && clamp(testInputs.xyz, vec4(-1.0, -2.0, -2.0, 1.0).xyz, vec4(1.0, 2.0, 0.5, 3.0).xyz) == vec4(-1.0, 0.0, 0.5, 2.25).xyz) && clamp(testInputs, vec4(-1.0, -2.0, -2.0, 1.0), vec4(1.0, 2.0, 0.5, 3.0)) == vec4(-1.0, 0.0, 0.5, 2.25) ? colorGreen : colorRed;
}
