
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bvec4 inputVal = bvec4(colorGreen.xxyz);
    bvec4 expected = bvec4(colorGreen.xyyw);
    return ((((any(inputVal.xy) == expected.x && any(inputVal.xyz) == expected.y) && any(inputVal) == expected.z) && false == expected.x) && expected.y) && expected.z ? colorGreen : colorRed;
}
