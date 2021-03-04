
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 intValues = ivec4(testInputs * 100.0);
    ivec4 intGreen = ivec4(colorGreen * 100.0);
    return ((((((max(intValues.x, 50) == 50 && max(intValues.xy, 50) == ivec2(50, 50)) && max(intValues.xyz, 50) == ivec3(50, 50, 75)) && max(intValues, 50) == ivec4(50, 50, 75, 225)) && max(intValues.x, intGreen.x) == 0) && max(intValues.xy, intGreen.xy) == ivec2(0, 100)) && max(intValues.xyz, intGreen.xyz) == ivec3(0, 100, 75)) && max(intValues, intGreen) == ivec4(0, 100, 75, 225) ? colorGreen : colorRed;
}
