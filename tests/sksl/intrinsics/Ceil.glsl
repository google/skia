
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((ceil(testInputs.x) == -1.0 && ceil(testInputs.xy) == vec2(-1.0, 0.0)) && ceil(testInputs.xyz) == vec3(-1.0, 0.0, 1.0)) && ceil(testInputs) == vec4(-1.0, 0.0, 1.0, 3.0) ? colorGreen : colorRed;
}
