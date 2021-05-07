
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((log(input.x) == expected.x && log(input.xy) == expected.xy) && log(input.xyz) == expected.xyz) && log(input) == expected) && -inf.0 == expected.x) && vec2(-inf.0, -inf.0) == expected.xy) && vec3(-inf.0, -inf.0, -inf.0) == expected.xyz) && vec4(-inf.0, -inf.0, -inf.0, -inf.0) == expected ? colorGreen : colorRed;
}
