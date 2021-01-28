
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 intValues = ivec4(testInputs * 100.0);
    ivec4 intGreen = ivec4(colorGreen * 100.0);
    return ((((((min(intValues.x, 50) == -125 && min(intValues.xy, 50) == ivec2(-125, 0)) && min(intValues.xyz, 50) == ivec3(-125, 0, 50)) && min(intValues, 50) == ivec4(-125, 0, 50, 50)) && min(intValues.x, intGreen.x) == -125) && min(intValues.xy, intGreen.xy) == ivec2(-125, 0)) && min(intValues.xyz, intGreen.xyz) == ivec3(-125, 0, 0)) && min(intValues, intGreen) == ivec4(-125, 0, 0, 100) ? colorGreen : colorRed;
}
