
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    uvec4 uintValues = uvec4(abs(testInputs) * 100.0);
    uvec4 uintGreen = uvec4(colorGreen * 100.0);
    uvec4 expectedA = uvec4(50u, 0u, 50u, 50u);
    uvec4 expectedB = uvec4(0u, 0u, 0u, 100u);
    return ((((((((((((((min(uintValues.x, 50u) == expectedA.x && min(uintValues.xy, 50u) == expectedA.xy) && min(uintValues.xyz, 50u) == expectedA.xyz) && min(uintValues, 50u) == expectedA) && 50u == expectedA.x) && uvec2(50u, 0u) == expectedA.xy) && uvec3(50u, 0u, 50u) == expectedA.xyz) && uvec4(50u, 0u, 50u, 50u) == expectedA) && min(uintValues.x, uintGreen.x) == expectedB.x) && min(uintValues.xy, uintGreen.xy) == expectedB.xy) && min(uintValues.xyz, uintGreen.xyz) == expectedB.xyz) && min(uintValues, uintGreen) == expectedB) && 0u == expectedB.x) && uvec2(0u) == expectedB.xy) && uvec3(0u) == expectedB.xyz) && uvec4(0u, 0u, 0u, 100u) == expectedB ? colorGreen : colorRed;
}
