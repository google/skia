
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorWhite;
vec4 main() {
    vec4 expectedA = vec4(0.75, 0.0, 0.75, 0.25);
    vec4 expectedB = vec4(0.25, 0.0, 0.75, 1.0);
    return ((((((((((((((mod(testInputs.x, 1.0) == expectedA.x && mod(testInputs.xy, 1.0) == expectedA.xy) && mod(testInputs.xyz, 1.0) == expectedA.xyz) && mod(testInputs, 1.0) == expectedA) && 0.75 == expectedA.x) && vec2(0.75, 0.0) == expectedA.xy) && vec3(0.75, 0.0, 0.75) == expectedA.xyz) && vec4(0.75, 0.0, 0.75, 0.25) == expectedA) && mod(testInputs.x, colorWhite.x) == expectedA.x) && mod(testInputs.xy, colorWhite.xy) == expectedA.xy) && mod(testInputs.xyz, colorWhite.xyz) == expectedA.xyz) && mod(testInputs, colorWhite) == expectedA) && 0.25 == expectedB.x) && vec2(0.25, 0.0) == expectedB.xy) && vec3(0.25, 0.0, 0.75) == expectedB.xyz) && vec4(0.25, 0.0, 0.75, 1.0) == expectedB ? colorGreen : colorRed;
}
