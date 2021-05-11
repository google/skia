
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((roundEven(input.x) == expected.x && roundEven(input.xy) == expected.xy) && roundEven(input.xyz) == expected.xyz) && roundEven(input) == expected) && -2.0 == expected.x) && vec2(-2.0, -0.0) == expected.xy) && vec3(-2.0, -0.0, 0.0) == expected.xyz) && vec4(-2.0, -0.0, 0.0, 2.0) == expected ? colorGreen : colorRed;
}
