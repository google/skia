
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((abs(testInputs.x) == 1.25 && abs(testInputs.xy) == vec2(1.25, 0.0)) && abs(testInputs.xyz) == vec3(1.25, 0.0, 0.75)) && abs(testInputs) == vec4(1.25, 0.0, 0.75, 2.25) ? colorGreen : colorRed;
}
