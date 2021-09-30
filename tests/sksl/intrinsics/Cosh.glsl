
out vec4 sk_FragColor;
uniform vec4 inputVal;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((cosh(inputVal.x) == expected.x && cosh(inputVal.xy) == expected.xy) && cosh(inputVal.xyz) == expected.xyz) && cosh(inputVal) == expected) && 1.0 == expected.x) && vec2(1.0, 1.0) == expected.xy) && vec3(1.0, 1.0, 1.0) == expected.xyz) && vec4(1.0, 1.0, 1.0, 1.0) == expected ? colorGreen : colorRed;
}
