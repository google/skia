
out vec4 sk_FragColor;
uniform float testInput;
uniform mat2 testMatrix2x2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 inputVal = vec4(testMatrix2x2) * vec4(1.0, 1.0, -1.0, -1.0);
    uvec4 expectedB = uvec4(1065353216u, 1073741824u, 3225419776u, 3229614080u);
    return ((inputVal.x == uintBitsToFloat(expectedB.x) && inputVal.xy == uintBitsToFloat(expectedB.xy)) && inputVal.xyz == uintBitsToFloat(expectedB.xyz)) && inputVal == uintBitsToFloat(expectedB) ? colorGreen : colorRed;
}
