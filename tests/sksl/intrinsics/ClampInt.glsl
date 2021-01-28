
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    ivec4 intValues = ivec4(testInputs * 100.0);
    return ((((((clamp(intValues.x, -100, 100) == -100 && clamp(intValues.xy, -100, 100) == ivec2(-100, 0)) && clamp(intValues.xyz, -100, 100) == ivec3(-100, 0, 75)) && clamp(intValues, -100, 100) == ivec4(-100, 0, 75, 100)) && clamp(intValues.x, -100, 100) == -100) && clamp(intValues.xy, ivec2(-100, -200), ivec2(100, 200)) == ivec2(-100, 0)) && clamp(intValues.xyz, ivec3(-100, -200, -200), ivec3(100, 200, 50)) == ivec3(-100, 0, 50)) && clamp(intValues, ivec4(-100, -200, -200, 100), ivec4(100, 200, 50, 300)) == ivec4(-100, 0, 50, 225) ? colorGreen : colorRed;
}
