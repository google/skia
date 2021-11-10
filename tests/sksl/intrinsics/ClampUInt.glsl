
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    uvec4 uintValues = uvec4(testInputs * 100.0 + 200.0);
    uvec4 expectedA = uvec4(100u, 200u, 275u, 300u);
    const uvec4 clampLow = uvec4(100u, 0u, 0u, 300u);
    uvec4 expectedB = uvec4(100u, 200u, 250u, 425u);
    const uvec4 clampHigh = uvec4(300u, 400u, 250u, 500u);
    return ((((((((((((((clamp(uintValues.x, 100u, 300u) == expectedA.x && clamp(uintValues.xy, 100u, 300u) == expectedA.xy) && clamp(uintValues.xyz, 100u, 300u) == expectedA.xyz) && clamp(uintValues, 100u, 300u) == expectedA) && 100u == expectedA.x) && uvec2(100u, 200u) == expectedA.xy) && uvec3(100u, 200u, 275u) == expectedA.xyz) && uvec4(100u, 200u, 275u, 300u) == expectedA) && clamp(uintValues.x, 100u, 300u) == expectedB.x) && clamp(uintValues.xy, uvec2(100u, 0u), uvec2(300u, 400u)) == expectedB.xy) && clamp(uintValues.xyz, uvec3(100u, 0u, 0u), uvec3(300u, 400u, 250u)) == expectedB.xyz) && clamp(uintValues, clampLow, clampHigh) == expectedB) && 100u == expectedB.x) && uvec2(100u, 200u) == expectedB.xy) && uvec3(100u, 200u, 250u) == expectedB.xyz) && uvec4(100u, 200u, 250u, 425u) == expectedB ? colorGreen : colorRed;
}
