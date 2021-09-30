
out vec4 sk_FragColor;
uniform vec4 inputVal;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((degrees(inputVal.x) == expected.x && degrees(inputVal.xy) == expected.xy) && degrees(inputVal.xyz) == expected.xyz) && degrees(inputVal) == expected) && 90.0 == expected.x) && vec2(90.0, 180.0) == expected.xy) && vec3(90.0, 180.0, 270.0) == expected.xyz) && vec4(90.0, 180.0, 270.0, 360.0) == expected ? colorGreen : colorRed;
}
