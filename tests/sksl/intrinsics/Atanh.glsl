
out vec4 sk_FragColor;
uniform vec4 inputVal;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((atanh(inputVal.x) == expected.x && atanh(inputVal.xy) == expected.xy) && atanh(inputVal.xyz) == expected.xyz) && atanh(inputVal) == expected) && 0.0 == expected.x) && vec2(0.0, 0.25) == expected.xy) && vec3(0.0, 0.25, 0.5) == expected.xyz) && vec4(0.0, 0.25, 0.5, 1.0) == expected ? colorGreen : colorRed;
}
