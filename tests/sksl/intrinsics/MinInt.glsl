
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 intValues = ivec4(testInputs * 100.0);
    ivec4 intGreen = ivec4(colorGreen * 100.0);
    ivec4 expectedA = ivec4(-125, 0, 50, 50);
    ivec4 expectedB = ivec4(-125, 0, 0, 100);
    return ((((((((((((((min(intValues.x, 50) == expectedA.x && min(intValues.xy, 50) == expectedA.xy) && min(intValues.xyz, 50) == expectedA.xyz) && min(intValues, 50) == expectedA) && -125 == expectedA.x) && ivec2(-125, 0) == expectedA.xy) && ivec3(-125, 0, 50) == expectedA.xyz) && ivec4(-125, 0, 50, 50) == expectedA) && min(intValues.x, intGreen.x) == expectedB.x) && min(intValues.xy, intGreen.xy) == expectedB.xy) && min(intValues.xyz, intGreen.xyz) == expectedB.xyz) && min(intValues, intGreen) == expectedB) && -125 == expectedB.x) && ivec2(-125, 0) == expectedB.xy) && ivec3(-125, 0, 0) == expectedB.xyz) && ivec4(-125, 0, 0, 100) == expectedB ? colorGreen : colorRed;
}
