
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 intValues = ivec4(testInputs * 100.0);
    ivec4 intGreen = ivec4(colorGreen * 100.0);
    ivec4 expectedA = ivec4(-125, 0, 50, 50);
    ivec4 expectedB = ivec4(-125, 0, 0, 100);
    return ((((((min(intValues.x, 50) == expectedA.x && min(intValues.xy, 50) == expectedA.xy) && min(intValues.xyz, 50) == expectedA.xyz) && min(intValues, 50) == expectedA) && min(intValues.x, intGreen.x) == expectedB.x) && min(intValues.xy, intGreen.xy) == expectedB.xy) && min(intValues.xyz, intGreen.xyz) == expectedB.xyz) && min(intValues, intGreen) == expectedB ? colorGreen : colorRed;
}
