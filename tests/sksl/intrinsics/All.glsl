
out vec4 sk_FragColor;
uniform vec4 inputH4;
uniform vec4 expectedH4;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bvec4 input = bvec4(inputH4);
    bvec4 expected = bvec4(expectedH4);
    return ((((all(input.xy) == expected.x && all(input.xyz) == expected.y) && all(input) == expected.z) && expected.x) && false == expected.y) && false == expected.z ? colorGreen : colorRed;
}
