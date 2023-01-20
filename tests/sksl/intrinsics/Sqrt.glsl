
out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 negativeVal = vec4(-1.0, -4.0, -16.0, -64.0);
    coords = sqrt(negativeVal).xy;
    vec4 inputVal = vec4(testMatrix2x2) + vec4(0.0, 2.0, 6.0, 12.0);
    const vec4 expected = vec4(1.0, 2.0, 3.0, 4.0);
    return ((sqrt(inputVal.x) == 1.0 && sqrt(inputVal.xy) == vec2(1.0, 2.0)) && sqrt(inputVal.xyz) == vec3(1.0, 2.0, 3.0)) && sqrt(inputVal) == expected ? colorGreen : colorRed;
}
