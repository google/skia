
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 negativeVal = vec4(-1.0, -4.0, -16.0, -64.0);
    return ((((((((((inversesqrt(input.x) == expected.x && inversesqrt(input.xy) == expected.xy) && inversesqrt(input.xyz) == expected.xyz) && inversesqrt(input) == expected) && 1.0 == expected.x) && vec2(1.0, 0.5) == expected.xy) && vec3(1.0, 0.5, 0.25) == expected.xyz) && vec4(1.0, 0.5, 0.25, 0.125) == expected) && inversesqrt(-1.0) == expected.x) && inversesqrt(vec2(-1.0, -4.0)) == expected.xy) && inversesqrt(vec3(-1.0, -4.0, -16.0)) == expected.xyz) && inversesqrt(negativeVal) == expected ? colorGreen : colorRed;
}
