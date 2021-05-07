
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(1.25, 0.0, 0.75, 2.25);
    return ((((((abs(testInputs.x) == expected.x && abs(testInputs.xy) == expected.xy) && abs(testInputs.xyz) == expected.xyz) && abs(testInputs) == expected) && 1.25 == expected.x) && vec2(1.25, 0.0) == expected.xy) && vec3(1.25, 0.0, 0.75) == expected.xyz) && vec4(1.25, 0.0, 0.75, 2.25) == expected ? colorGreen : colorRed;
}
