
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expectedA = vec4(-1.0, 0.0, 0.75, 1.0);
    vec4 clampLow = vec4(-1.0, -2.0, -2.0, 1.0);
    vec4 expectedB = vec4(-1.0, 0.0, 0.5, 2.25);
    vec4 clampHigh = vec4(1.0, 2.0, 0.5, 3.0);
    return ((((((clamp(testInputs.x, -1.0, 1.0) == expectedA.x && clamp(testInputs.xy, -1.0, 1.0) == expectedA.xy) && clamp(testInputs.xyz, -1.0, 1.0) == expectedA.xyz) && clamp(testInputs, -1.0, 1.0) == expectedA) && clamp(testInputs.x, clampLow.x, clampHigh.x) == expectedB.x) && clamp(testInputs.xy, clampLow.xy, clampHigh.xy) == expectedB.xy) && clamp(testInputs.xyz, clampLow.xyz, clampHigh.xyz) == expectedB.xyz) && clamp(testInputs, clampLow, clampHigh) == expectedB ? colorGreen : colorRed;
}
