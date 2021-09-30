
out vec4 sk_FragColor;
uniform vec4 inputVal;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((exp2(inputVal.x) == expected.x && exp2(inputVal.xy) == expected.xy) && exp2(inputVal.xyz) == expected.xyz) && exp2(inputVal) == expected) && 1.0 == expected.x) && vec2(1.0, 2.0) == expected.xy) && vec3(1.0, 2.0, 4.0) == expected.xyz) && vec4(1.0, 2.0, 4.0, 8.0) == expected ? colorGreen : colorRed;
}
