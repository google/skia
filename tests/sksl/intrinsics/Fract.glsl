
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 expected = vec4(0.75, 0.0, 0.75, 0.25);
    return ((fract(testInputs.x) == 0.75 && fract(testInputs.xy) == vec2(0.75, 0.0)) && fract(testInputs.xyz) == vec3(0.75, 0.0, 0.75)) && fract(testInputs) == expected ? colorGreen : colorRed;
}
