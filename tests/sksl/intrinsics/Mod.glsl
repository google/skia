
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorWhite;
vec4 main() {
    vec4 vector2 = 2.0 * colorWhite;
    return ((((((mod(testInputs.x, 2.0) == 0.75 && mod(testInputs.xy, 2.0) == vec2(0.75, 0.0)) && mod(testInputs.xyz, 2.0) == vec3(0.75, 0.0, 0.75)) && mod(testInputs, 2.0) == vec4(0.75, 0.0, 0.75, 0.25)) && mod(testInputs.x, vector2.x) == 0.75) && mod(testInputs.xy, vector2.xy) == vec2(0.75, 0.0)) && mod(testInputs.xyz, vector2.xyz) == vec3(0.75, 0.0, 0.75)) && mod(testInputs, vector2) == vec4(0.75, 0.0, 0.75, 0.25) ? colorGreen : colorRed;
}
