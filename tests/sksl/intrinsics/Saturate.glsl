
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(0.0, 0.0, 0.75, 1.0);
    return ((((((clamp(testInputs.x, 0.0, 1.0) == expected.x && clamp(testInputs.xy, 0.0, 1.0) == expected.xy) && clamp(testInputs.xyz, 0.0, 1.0) == expected.xyz) && clamp(testInputs, 0.0, 1.0) == expected) && 0.0 == expected.x) && vec2(0.0) == expected.xy) && vec3(0.0, 0.0, 0.75) == expected.xyz) && vec4(0.0, 0.0, 0.75, 1.0) == expected ? colorGreen : colorRed;
}
