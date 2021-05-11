
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((round(input.x) == expected.x && round(input.xy) == expected.xy) && round(input.xyz) == expected.xyz) && round(input) == expected) && -2.0 == expected.x) && vec2(-2.0, -0.0) == expected.xy) && vec3(-2.0, -0.0, 0.0) == expected.xyz) && vec4(-2.0, -0.0, 0.0, 2.0) == expected ? colorGreen : colorRed;
}
