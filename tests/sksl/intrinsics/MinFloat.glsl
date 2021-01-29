
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((min(testInputs.x, 0.5) == -1.25 && min(testInputs.xy, 0.5) == vec2(-1.25, 0.0)) && min(testInputs.xyz, 0.5) == vec3(-1.25, 0.0, 0.5)) && min(testInputs, 0.5) == vec4(-1.25, 0.0, 0.5, 0.5)) && min(testInputs.x, colorGreen.x) == -1.25) && min(testInputs.xy, colorGreen.xy) == vec2(-1.25, 0.0)) && min(testInputs.xyz, colorGreen.xyz) == vec3(-1.25, 0.0, 0.0)) && min(testInputs, colorGreen) == vec4(-1.25, 0.0, 0.0, 1.0) ? colorGreen : colorRed;
}
