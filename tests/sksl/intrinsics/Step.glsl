
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 constGreen = vec4(0.0, 1.0, 0.0, 1.0);
    vec4 expectedA = vec4(0.0, 0.0, 1.0, 1.0);
    vec4 expectedB = vec4(1.0, 1.0, 0.0, 0.0);
    vec4 expectedC = vec4(0.0, 1.0, 1.0, 1.0);
    return ((((((((((((((((((((((step(0.5, testInputs.x) == expectedA.x && step(0.5, testInputs.xy) == expectedA.xy) && step(0.5, testInputs.xyz) == expectedA.xyz) && step(0.5, testInputs) == expectedA) && 0.0 == expectedA.x) && vec2(0.0) == expectedA.xy) && vec3(0.0, 0.0, 1.0) == expectedA.xyz) && vec4(0.0, 0.0, 1.0, 1.0) == expectedA) && step(testInputs.x, 0.0) == expectedB.x) && step(testInputs.xy, vec2(0.0, 1.0)) == expectedB.xy) && step(testInputs.xyz, vec3(0.0, 1.0, 0.0)) == expectedB.xyz) && step(testInputs, constGreen) == expectedB) && 1.0 == expectedB.x) && vec2(1.0) == expectedB.xy) && vec3(1.0, 1.0, 0.0) == expectedB.xyz) && vec4(1.0, 1.0, 0.0, 0.0) == expectedB) && step(colorRed.x, colorGreen.x) == expectedC.x) && step(colorRed.xy, colorGreen.xy) == expectedC.xy) && step(colorRed.xyz, colorGreen.xyz) == expectedC.xyz) && step(colorRed, colorGreen) == expectedC) && 0.0 == expectedC.x) && vec2(0.0, 1.0) == expectedC.xy) && vec3(0.0, 1.0, 1.0) == expectedC.xyz) && vec4(0.0, 1.0, 1.0, 1.0) == expectedC ? colorGreen : colorRed;
}
