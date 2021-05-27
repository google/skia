
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 constVal2 = vec4(1.0);
    return ((((((((((((((atan(input.x) == expected.x && atan(input.xy) == expected.xy) && atan(input.xyz) == expected.xyz) && atan(input) == expected) && 0.0 == expected.x) && vec2(0.0, 0.0) == expected.xy) && vec3(0.0, 0.0, 0.0) == expected.xyz) && vec4(0.0, 0.0, 0.0, 0.0) == expected) && atan(input.x, 1.0) == expected.x) && atan(input.xy, vec2(1.0)) == expected.xy) && atan(input.xyz, vec3(1.0)) == expected.xyz) && atan(input, constVal2) == expected) && 0.0 == expected.x) && vec2(0.0, 0.0) == expected.xy) && vec3(0.0, 0.0, 0.0) == expected.xyz) && vec4(0.0, 0.0, 0.0, 0.0) == expected ? colorGreen : colorRed;
}
