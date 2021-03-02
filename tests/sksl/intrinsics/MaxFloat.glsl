
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expectedA = vec4(0.5, 0.5, 0.75, 2.25);
    vec4 expectedB = vec4(0.0, 1.0, 0.75, 2.25);
    return ((((((max(testInputs.x, 0.5) == expectedA.x && max(testInputs.xy, 0.5) == expectedA.xy) && max(testInputs.xyz, 0.5) == expectedA.xyz) && max(testInputs, 0.5) == expectedA) && max(testInputs.x, colorGreen.x) == expectedB.x) && max(testInputs.xy, colorGreen.xy) == expectedB.xy) && max(testInputs.xyz, colorGreen.xyz) == expectedB.xyz) && max(testInputs, colorGreen) == expectedB ? colorGreen : colorRed;
}
