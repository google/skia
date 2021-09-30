
out vec4 sk_FragColor;
uniform vec4 inputVal;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((acos(inputVal.x) == expected.x && acos(inputVal.xy) == expected.xy) && acos(inputVal.xyz) == expected.xyz) && acos(inputVal) == expected) && 0.0 == expected.x) && vec2(0.0, 0.0) == expected.xy) && vec3(0.0, 0.0, 0.0) == expected.xyz) && vec4(0.0, 0.0, 0.0, 0.0) == expected ? colorGreen : colorRed;
}
