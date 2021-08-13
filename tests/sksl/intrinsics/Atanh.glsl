
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((atanh(input.x) == expected.x && atanh(input.xy) == expected.xy) && atanh(input.xyz) == expected.xyz) && atanh(input) == expected) && 0.0 == expected.x) && vec2(0.0, 0.25) == expected.xy) && vec3(0.0, 0.25, 0.5) == expected.xyz) && vec4(0.0, 0.25, 0.5, 1.0) == expected ? colorGreen : colorRed;
}
