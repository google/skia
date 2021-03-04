
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((max(testInputs.x, 0.5) == 0.5 && max(testInputs.xy, 0.5) == vec2(0.5, 0.5)) && max(testInputs.xyz, 0.5) == vec3(0.5, 0.5, 0.75)) && max(testInputs, 0.5) == vec4(0.5, 0.5, 0.75, 2.25)) && max(testInputs.x, colorGreen.x) == 0.0) && max(testInputs.xy, colorGreen.xy) == vec2(0.0, 1.0)) && max(testInputs.xyz, colorGreen.xyz) == vec3(0.0, 1.0, 0.75)) && max(testInputs, colorGreen) == vec4(0.0, 1.0, 0.75, 2.25) ? colorGreen : colorRed;
}
