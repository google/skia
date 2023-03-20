
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 intValues = ivec4(testInputs * 100.0);
    ivec4 intGreen = ivec4(colorGreen * 100.0);
    ivec4 expectedA = ivec4(50, 50, 75, 225);
    ivec4 expectedB = ivec4(0, 100, 75, 225);
    return ((((((((((((((max(intValues.x, 50) == expectedA.x && max(intValues.xy, 50) == expectedA.xy) && max(intValues.xyz, 50) == expectedA.xyz) && max(intValues, 50) == expectedA) && 50 == expectedA.x) && ivec2(50) == expectedA.xy) && ivec3(50, 50, 75) == expectedA.xyz) && ivec4(50, 50, 75, 225) == expectedA) && max(intValues.x, intGreen.x) == expectedB.x) && max(intValues.xy, intGreen.xy) == expectedB.xy) && max(intValues.xyz, intGreen.xyz) == expectedB.xyz) && max(intValues, intGreen) == expectedB) && 0 == expectedB.x) && ivec2(0, 100) == expectedB.xy) && ivec3(0, 100, 75) == expectedB.xyz) && ivec4(0, 100, 75, 225) == expectedB ? colorGreen : colorRed;
}
