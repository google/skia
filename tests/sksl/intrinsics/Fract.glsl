
out vec4 sk_FragColor;
uniform vec4 inputVal;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((fract(inputVal.x) == expected.x && fract(inputVal.xy) == expected.xy) && fract(inputVal.xyz) == expected.xyz) && fract(inputVal) == expected) && 0.0 == expected.x) && vec2(0.0, 0.25) == expected.xy) && vec3(0.0, 0.25, 0.5) == expected.xyz) && vec4(0.0, 0.25, 0.5, 0.75) == expected ? colorGreen : colorRed;
}
