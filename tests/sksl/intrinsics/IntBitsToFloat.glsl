
out vec4 sk_FragColor;
uniform float testInput;
uniform mat2 testMatrix2x2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 inputVal = vec4(testMatrix2x2) * vec4(1.0, 1.0, -1.0, -1.0);
    ivec4 expectedB = ivec4(1065353216, 1073741824, -1069547520, -1065353216);
    return ((inputVal.x == intBitsToFloat(expectedB.x) && inputVal.xy == intBitsToFloat(expectedB.xy)) && inputVal.xyz == intBitsToFloat(expectedB.xyz)) && inputVal == intBitsToFloat(expectedB) ? colorGreen : colorRed;
}
