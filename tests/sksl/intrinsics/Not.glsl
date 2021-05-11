
out vec4 sk_FragColor;
uniform vec4 inputH4;
uniform vec4 expectedH4;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bvec4 input = bvec4(inputH4);
    bvec4 expected = bvec4(expectedH4);
    return ((((not(input.xy) == expected.xy && not(input.xyz) == expected.xyz) && not(input) == expected) && bvec2(false, true) == expected.xy) && bvec3(false, true, false) == expected.xyz) && bvec4(false, true, false, true) == expected ? colorGreen : colorRed;
}
