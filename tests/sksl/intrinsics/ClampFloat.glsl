
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((clamp(testInputs.x, -1.0, 1.0) == -1.0 && clamp(testInputs.xy, -1.0, 1.0) == vec2(-1.0, 0.0)) && clamp(testInputs.xyz, -1.0, 1.0) == vec3(-1.0, 0.0, 0.75)) && clamp(testInputs, -1.0, 1.0) == vec4(-1.0, 0.0, 0.75, 1.0)) && clamp(testInputs.x, -1.0, 1.0) == -1.0) && clamp(testInputs.xy, vec2(-1.0, -2.0), vec2(1.0, 2.0)) == vec2(-1.0, 0.0)) && clamp(testInputs.xyz, vec3(-1.0, -2.0, -2.0), vec3(1.0, 2.0, 0.5)) == vec3(-1.0, 0.0, 0.5)) && clamp(testInputs, vec4(-1.0, -2.0, -2.0, 1.0), vec4(1.0, 2.0, 0.5, 3.0)) == vec4(-1.0, 0.0, 0.5, 2.25) ? colorGreen : colorRed;
}
