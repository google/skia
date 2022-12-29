
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    uvec4 uintValues = uvec4(abs(testInputs) * 100.0);
    uvec4 uintGreen = uvec4(colorGreen * 100.0);
    uvec4 expectedA = uvec4(125u, 80u, 80u, 225u);
    uvec4 expectedB = uvec4(125u, 100u, 75u, 225u);
    return ((((((((((((((max(uintValues.x, 80u) == expectedA.x && max(uintValues.xy, 80u) == expectedA.xy) && max(uintValues.xyz, 80u) == expectedA.xyz) && max(uintValues, 80u) == expectedA) && 125u == expectedA.x) && uvec2(125u, 80u) == expectedA.xy) && uvec3(125u, 80u, 80u) == expectedA.xyz) && uvec4(125u, 80u, 80u, 225u) == expectedA) && max(uintValues.x, uintGreen.x) == expectedB.x) && max(uintValues.xy, uintGreen.xy) == expectedB.xy) && max(uintValues.xyz, uintGreen.xyz) == expectedB.xyz) && max(uintValues, uintGreen) == expectedB) && 125u == expectedB.x) && uvec2(125u, 100u) == expectedB.xy) && uvec3(125u, 100u, 75u) == expectedB.xyz) && uvec4(125u, 100u, 75u, 225u) == expectedB ? colorGreen : colorRed;
}
