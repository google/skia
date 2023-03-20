
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bvec4 inputVal = bvec4(colorGreen);
    bvec4 expected = bvec4(true, false, true, false);
    return ((((not(inputVal.xy) == expected.xy && not(inputVal.xyz) == expected.xyz) && not(inputVal) == expected) && bvec2(true, false) == expected.xy) && bvec3(true, false, true) == expected.xyz) && bvec4(true, false, true, false) == expected ? colorGreen : colorRed;
}
