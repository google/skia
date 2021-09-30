
out vec4 sk_FragColor;
uniform vec4 inputVal;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 negativeVal = vec4(-1.0, -4.0, -16.0, -64.0);
    return ((((((((((sqrt(inputVal.x) == expected.x && sqrt(inputVal.xy) == expected.xy) && sqrt(inputVal.xyz) == expected.xyz) && sqrt(inputVal) == expected) && 1.0 == expected.x) && vec2(1.0, 2.0) == expected.xy) && vec3(1.0, 2.0, 4.0) == expected.xyz) && vec4(1.0, 2.0, 4.0, 8.0) == expected) && sqrt(-1.0) == expected.x) && sqrt(vec2(-1.0, -4.0)) == expected.xy) && sqrt(vec3(-1.0, -4.0, -16.0)) == expected.xyz) && sqrt(negativeVal) == expected ? colorGreen : colorRed;
}
