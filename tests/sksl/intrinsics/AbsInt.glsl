
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 expected = ivec4(1, 0, 0, 2);
    return ((abs(int(testInputs.x)) == expected.x && abs(ivec2(testInputs.xy)) == expected.xy) && abs(ivec3(testInputs.xyz)) == expected.xyz) && abs(ivec4(testInputs)) == expected ? colorGreen : colorRed;
}
