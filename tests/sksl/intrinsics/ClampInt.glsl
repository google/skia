
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 expectedA = ivec4(-100, 0, 75, 100);
    ivec4 intValues = ivec4(testInputs * 100.0);
    ivec4 clampLow = ivec4(-100, -200, -200, 100);
    ivec4 expectedB = ivec4(-100, 0, 50, 225);
    ivec4 clampHigh = ivec4(100, 200, 50, 300);
    return ((((((clamp(intValues.x, -100, 100) == expectedA.x && clamp(intValues.xy, -100, 100) == expectedA.xy) && clamp(intValues.xyz, -100, 100) == expectedA.xyz) && clamp(intValues, -100, 100) == expectedA) && clamp(intValues.x, clampLow.x, clampHigh.x) == expectedB.x) && clamp(intValues.xy, clampLow.xy, clampHigh.xy) == expectedB.xy) && clamp(intValues.xyz, clampLow.xyz, clampHigh.xyz) == expectedB.xyz) && clamp(intValues, clampLow, clampHigh) == expectedB ? colorGreen : colorRed;
}
