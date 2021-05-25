
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expectedA = vec4(-1.25, 0.0, 0.5, 0.5);
    vec4 expectedB = vec4(-1.25, 0.0, 0.0, 1.0);
    return ((((((((((((((min(testInputs.x, 0.5) == expectedA.x && min(testInputs.xy, 0.5) == expectedA.xy) && min(testInputs.xyz, 0.5) == expectedA.xyz) && min(testInputs, 0.5) == expectedA) && -1.25 == expectedA.x) && vec2(-1.25, 0.0) == expectedA.xy) && vec3(-1.25, 0.0, 0.5) == expectedA.xyz) && vec4(-1.25, 0.0, 0.5, 0.5) == expectedA) && min(testInputs.x, colorGreen.x) == expectedB.x) && min(testInputs.xy, colorGreen.xy) == expectedB.xy) && min(testInputs.xyz, colorGreen.xyz) == expectedB.xyz) && min(testInputs, colorGreen) == expectedB) && -1.25 == expectedB.x) && vec2(-1.25, 0.0) == expectedB.xy) && vec3(-1.25, 0.0, 0.0) == expectedB.xyz) && vec4(-1.25, 0.0, 0.0, 1.0) == expectedB ? colorGreen : colorRed;
}
