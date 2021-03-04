
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((abs(int(testInputs.x)) == 1 && abs(ivec2(testInputs.xy)) == ivec2(1, 0)) && abs(ivec3(testInputs.xyz)) == ivec3(1, 0, 0)) && abs(ivec4(testInputs)) == ivec4(1, 0, 0, 2) ? colorGreen : colorRed;
}
