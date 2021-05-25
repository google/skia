
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec3 expected = vec3(3.0, 5.0, 13.0);
    return ((((length(input.xy) == expected.x && length(input.xyz) == expected.y) && length(input) == expected.z) && 3.0 == expected.x) && 5.0 == expected.y) && 13.0 == expected.z ? colorGreen : colorRed;
}
