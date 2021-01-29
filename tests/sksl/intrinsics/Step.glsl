
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorWhite;
vec4 main() {
    return ((((((step(1.0, testInputs.x) == 0.0 && step(1.0, testInputs.xy) == vec2(0.0, 0.0)) && step(1.0, testInputs.xyz) == vec3(0.0, 0.0, 0.0)) && step(1.0, testInputs) == vec4(0.0, 0.0, 0.0, 1.0)) && step(colorWhite.x, testInputs.x) == 0.0) && step(colorWhite.xy, testInputs.xy) == vec2(0.0, 0.0)) && step(colorWhite.xyz, testInputs.xyz) == vec3(0.0, 0.0, 0.0)) && step(colorWhite, testInputs) == vec4(0.0, 0.0, 0.0, 1.0) ? colorGreen : colorRed;
}
