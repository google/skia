
out vec4 sk_FragColor;
uniform float testInput;
uniform mat2 testMatrix2x2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 input = vec4(testMatrix2x2) * vec4(1.0, 1.0, -1.0, -1.0);
    const ivec4 expectedB = ivec4(1065353216, 1073741824, -1069547520, -1065353216);
    return ((floatBitsToInt(input.x) == 1065353216 && floatBitsToInt(input.xy) == ivec2(1065353216, 1073741824)) && floatBitsToInt(input.xyz) == ivec3(1065353216, 1073741824, -1069547520)) && floatBitsToInt(input) == expectedB ? colorGreen : colorRed;
}
