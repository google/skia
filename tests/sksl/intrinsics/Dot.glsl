
out vec4 sk_FragColor;
uniform vec4 inputA;
uniform vec4 inputB;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(5.0, 17.0, 38.0, 70.0);
    return ((((((dot(inputA.x, inputB.x) == expected.x && dot(inputA.xy, inputB.xy) == expected.y) && dot(inputA.xyz, inputB.xyz) == expected.z) && dot(inputA, inputB) == expected.w) && 5.0 == expected.x) && 17.0 == expected.y) && 38.0 == expected.z) && 70.0 == expected.w ? colorGreen : colorRed;
}
