
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorBlack;
uniform vec4 colorWhite;
uniform vec4 testInputs;
vec4 main() {
    vec4 expectedBW = vec4(0.5, 0.5, 0.5, 1.0);
    vec4 expectedWT = vec4(1.0, 0.5, 1.0, 2.25);
    return ((((((((((((((((((mix(colorGreen, colorRed, 0.0) == vec4(0.0, 1.0, 0.0, 1.0) && mix(colorGreen, colorRed, 0.25) == vec4(0.25, 0.75, 0.0, 1.0)) && mix(colorGreen, colorRed, 0.75) == vec4(0.75, 0.25, 0.0, 1.0)) && mix(colorGreen, colorRed, 1.0) == vec4(1.0, 0.0, 0.0, 1.0)) && mix(colorBlack.x, colorWhite.x, 0.5) == expectedBW.x) && mix(colorBlack.xy, colorWhite.xy, 0.5) == expectedBW.xy) && mix(colorBlack.xyz, colorWhite.xyz, 0.5) == expectedBW.xyz) && mix(colorBlack, colorWhite, 0.5) == expectedBW) && 0.5 == expectedBW.x) && vec2(0.5, 0.5) == expectedBW.xy) && vec3(0.5, 0.5, 0.5) == expectedBW.xyz) && vec4(0.5, 0.5, 0.5, 1.0) == expectedBW) && mix(colorWhite.x, testInputs.x, 0.0) == expectedWT.x) && mix(colorWhite.xy, testInputs.xy, vec2(0.0, 0.5)) == expectedWT.xy) && mix(colorWhite.xyz, testInputs.xyz, vec3(0.0, 0.5, 0.0)) == expectedWT.xyz) && mix(colorWhite, testInputs, vec4(0.0, 0.5, 0.0, 1.0)) == expectedWT) && 1.0 == expectedWT.x) && vec2(1.0, 0.5) == expectedWT.xy) && vec3(1.0, 0.5, 1.0) == expectedWT.xyz) && vec4(1.0, 0.5, 1.0, 2.25) == expectedWT ? colorGreen : colorRed;
}
