
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 expected = ivec4(-1, 0, 0, 1);
    return ((((((sign(int(testInputs.x)) == expected.x && sign(ivec2(testInputs.xy)) == expected.xy) && sign(ivec3(testInputs.xyz)) == expected.xyz) && sign(ivec4(testInputs)) == expected) && -1 == expected.x) && ivec2(-1, 0) == expected.xy) && ivec3(-1, 0, 0) == expected.xyz) && ivec4(-1, 0, 0, 1) == expected ? colorGreen : colorRed;
}
