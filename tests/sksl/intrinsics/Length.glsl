
out vec4 sk_FragColor;
uniform vec4 inputVal;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(3.0, 3.0, 5.0, 13.0);
    return ((((((length(inputVal.x) == expected.x && length(inputVal.xy) == expected.y) && length(inputVal.xyz) == expected.z) && length(inputVal) == expected.w) && 3.0 == expected.x) && 3.0 == expected.y) && 5.0 == expected.z) && 13.0 == expected.w ? colorGreen : colorRed;
}
