
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(3.0, 3.0, 5.0, 13.0);
    return ((((((length(input.x) == expected.x && length(input.xy) == expected.y) && length(input.xyz) == expected.z) && length(input) == expected.w) && 3.0 == expected.x) && 3.0 == expected.y) && 5.0 == expected.z) && 13.0 == expected.w ? colorGreen : colorRed;
}
