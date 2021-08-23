
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 expectedA = vec4(-1.0, 0.0, 1.0, 2.0);
    return ((roundEven(testInputs.x) == -1.0 && roundEven(testInputs.xy) == vec2(-1.0, 0.0)) && roundEven(testInputs.xyz) == vec3(-1.0, 0.0, 1.0)) && roundEven(testInputs) == expectedA ? colorGreen : colorRed;
}
