
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 intValues = ivec4(testInputs * 100.0);
    return ((((((clamp(intValues.x, -100, 100) == ivec4(-100, 0, 75, 100).x && clamp(intValues.xy, -100, 100) == ivec4(-100, 0, 75, 100).xy) && clamp(intValues.xyz, -100, 100) == ivec4(-100, 0, 75, 100).xyz) && clamp(intValues, -100, 100) == ivec4(-100, 0, 75, 100)) && clamp(intValues.x, ivec4(-100, -200, -200, 100).x, ivec4(100, 200, 50, 300).x) == ivec4(-100, 0, 50, 225).x) && clamp(intValues.xy, ivec4(-100, -200, -200, 100).xy, ivec4(100, 200, 50, 300).xy) == ivec4(-100, 0, 50, 225).xy) && clamp(intValues.xyz, ivec4(-100, -200, -200, 100).xyz, ivec4(100, 200, 50, 300).xyz) == ivec4(-100, 0, 50, 225).xyz) && clamp(intValues, ivec4(-100, -200, -200, 100), ivec4(100, 200, 50, 300)) == ivec4(-100, 0, 50, 225) ? colorGreen : colorRed;
}
