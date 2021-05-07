
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((trunc(input.x) == expected.x && trunc(input.xy) == expected.xy) && trunc(input.xyz) == expected.xyz) && trunc(input) == expected) && -1.0 == expected.x) && vec2(-1.0, -0.0) == expected.xy) && vec3(-1.0, -0.0, 0.0) == expected.xyz) && vec4(-1.0, -0.0, 0.0, 1.0) == expected ? colorGreen : colorRed;
}
