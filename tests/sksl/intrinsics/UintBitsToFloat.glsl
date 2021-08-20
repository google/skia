
out vec4 sk_FragColor;
uniform float testInput;
uniform mat2 testMatrix2x2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 input = vec4(testMatrix2x2) * vec4(1.0, 1.0, -1.0, -1.0);
    uvec4 expectedB = uvec4(1065353216u, 1073741824u, 3225419776u, 3229614080u);
    return ((input.x == uintBitsToFloat(expectedB.x) && input.xy == uintBitsToFloat(expectedB.xy)) && input.xyz == uintBitsToFloat(expectedB.xyz)) && input == uintBitsToFloat(expectedB) ? colorGreen : colorRed;
}
