
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((floor(testInputs.x) == -2.0 && floor(testInputs.xy) == vec2(-2.0, 0.0)) && floor(testInputs.xyz) == vec3(-2.0, 0.0, 0.0)) && floor(testInputs) == vec4(-2.0, 0.0, 0.0, 2.0) ? colorGreen : colorRed;
}
