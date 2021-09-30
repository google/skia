
out vec4 sk_FragColor;
uniform vec4 inputH4;
uniform vec4 expectedH4;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bvec4 inputVal = bvec4(inputH4);
    bvec4 expected = bvec4(expectedH4);
    return ((((not(inputVal.xy) == expected.xy && not(inputVal.xyz) == expected.xyz) && not(inputVal) == expected) && bvec2(false, true) == expected.xy) && bvec3(false, true, false) == expected.xyz) && bvec4(false, true, false, true) == expected ? colorGreen : colorRed;
}
