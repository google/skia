
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((sign(testInputs.x) == -1.0 && sign(testInputs.xy) == vec2(-1.0, 0.0)) && sign(testInputs.xyz) == vec3(-1.0, 0.0, 1.0)) && sign(testInputs) == vec4(-1.0, 0.0, 1.0, 1.0) ? colorGreen : colorRed;
}
