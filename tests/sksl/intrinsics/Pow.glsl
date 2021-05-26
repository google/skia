
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(-1.5625, 0.0, 0.75, 3.375);
    const vec4 exponents = vec4(2.0, 3.0, 1.0, 1.5);
    return ((((((pow(testInputs.x, 2.0) == expected.x && pow(testInputs.xy, vec2(2.0, 3.0)) == expected.xy) && pow(testInputs.xyz, vec3(2.0, 3.0, 1.0)) == expected.xyz) && pow(testInputs, exponents) == expected) && 1.5625 == expected.x) && vec2(1.5625, 0.0) == expected.xy) && vec3(1.5625, 0.0, 0.75) == expected.xyz) && vec4(1.5625, 0.0, 0.75, 3.375) == expected ? colorGreen : colorRed;
}
