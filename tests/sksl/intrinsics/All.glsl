
out vec4 sk_FragColor;
uniform vec4 inputH4;
uniform vec4 expectedH4;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bvec4 inputVal = bvec4(inputH4);
    bvec4 expected = bvec4(expectedH4);
    return ((((all(inputVal.xy) == expected.x && all(inputVal.xyz) == expected.y) && all(inputVal) == expected.z) && expected.x) && false == expected.y) && false == expected.z ? colorGreen : colorRed;
}
