
out vec4 sk_FragColor;
uniform vec4 inputVal;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 constVal2 = vec4(1.0);
    return ((((((((((((((atan(inputVal.x) == expected.x && atan(inputVal.xy) == expected.xy) && atan(inputVal.xyz) == expected.xyz) && atan(inputVal) == expected) && 0.0 == expected.x) && vec2(0.0, 0.0) == expected.xy) && vec3(0.0, 0.0, 0.0) == expected.xyz) && vec4(0.0, 0.0, 0.0, 0.0) == expected) && atan(inputVal.x, 1.0) == expected.x) && atan(inputVal.xy, vec2(1.0)) == expected.xy) && atan(inputVal.xyz, vec3(1.0)) == expected.xyz) && atan(inputVal, constVal2) == expected) && 0.0 == expected.x) && vec2(0.0, 0.0) == expected.xy) && vec3(0.0, 0.0, 0.0) == expected.xyz) && vec4(0.0, 0.0, 0.0, 0.0) == expected ? colorGreen : colorRed;
}
