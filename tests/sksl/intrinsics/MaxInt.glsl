
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 intValues = ivec4(testInputs * 100.0);
    ivec4 intGreen = ivec4(colorGreen * 100.0);
    return ((((((max(intValues.x, 50) == ivec4(50, 50, 75, 225).x && max(intValues.xy, 50) == ivec4(50, 50, 75, 225).xy) && max(intValues.xyz, 50) == ivec4(50, 50, 75, 225).xyz) && max(intValues, 50) == ivec4(50, 50, 75, 225)) && max(intValues.x, intGreen.x) == ivec4(0, 100, 75, 225).x) && max(intValues.xy, intGreen.xy) == ivec4(0, 100, 75, 225).xy) && max(intValues.xyz, intGreen.xyz) == ivec4(0, 100, 75, 225).xyz) && max(intValues, intGreen) == ivec4(0, 100, 75, 225) ? colorGreen : colorRed;
}
