
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 intValues = ivec4(testInputs * 100.0);
    ivec4 expectedA = ivec4(-100, 0, 75, 100);
    const ivec4 clampLow = ivec4(-100, -200, -200, 100);
    ivec4 expectedB = ivec4(-100, 0, 50, 225);
    const ivec4 clampHigh = ivec4(100, 200, 50, 300);
    return ((((((((((((((clamp(intValues.x, -100, 100) == expectedA.x && clamp(intValues.xy, -100, 100) == expectedA.xy) && clamp(intValues.xyz, -100, 100) == expectedA.xyz) && clamp(intValues, -100, 100) == expectedA) && -100 == expectedA.x) && ivec2(-100, 0) == expectedA.xy) && ivec3(-100, 0, 75) == expectedA.xyz) && ivec4(-100, 0, 75, 100) == expectedA) && clamp(intValues.x, -100, 100) == expectedB.x) && clamp(intValues.xy, ivec2(-100, -200), ivec2(100, 200)) == expectedB.xy) && clamp(intValues.xyz, ivec3(-100, -200, -200), ivec3(100, 200, 50)) == expectedB.xyz) && clamp(intValues, clampLow, clampHigh) == expectedB) && -100 == expectedB.x) && ivec2(-100, 0) == expectedB.xy) && ivec3(-100, 0, 50) == expectedB.xyz) && ivec4(-100, 0, 50, 225) == expectedB ? colorGreen : colorRed;
}
