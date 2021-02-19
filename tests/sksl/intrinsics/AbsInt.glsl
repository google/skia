
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((abs(int(testInputs.x)) == ivec4(1, 0, 0, 2).x && abs(ivec2(testInputs.xy)) == ivec4(1, 0, 0, 2).xy) && abs(ivec3(testInputs.xyz)) == ivec4(1, 0, 0, 2).xyz) && abs(ivec4(testInputs)) == ivec4(1, 0, 0, 2) ? colorGreen : colorRed;
}
