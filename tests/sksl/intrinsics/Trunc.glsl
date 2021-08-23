
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 expectedA = vec4(-1.0, 0.0, 0.0, 2.0);
    return ((trunc(testInputs.x) == -1.0 && trunc(testInputs.xy) == vec2(-1.0, 0.0)) && trunc(testInputs.xyz) == vec3(-1.0, 0.0, 0.0)) && trunc(testInputs) == expectedA ? colorGreen : colorRed;
}
