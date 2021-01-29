
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorBlack;
uniform vec4 colorWhite;
uniform vec4 testInputs;
vec4 main() {
    return ((((((((((mix(colorGreen, colorRed, 0.0) == vec4(0.0, 1.0, 0.0, 1.0) && mix(colorGreen, colorRed, 0.25) == vec4(0.25, 0.75, 0.0, 1.0)) && mix(colorGreen, colorRed, 0.75) == vec4(0.75, 0.25, 0.0, 1.0)) && mix(colorGreen, colorRed, 1.0) == vec4(1.0, 0.0, 0.0, 1.0)) && mix(colorBlack.x, colorWhite.x, 0.5) == 0.5) && mix(colorBlack.xy, colorWhite.xy, 0.5) == vec2(0.5, 0.5)) && mix(colorBlack.xyz, colorWhite.xyz, 0.5) == vec3(0.5, 0.5, 0.5)) && mix(colorBlack, colorWhite, 0.5) == vec4(0.5, 0.5, 0.5, 1.0)) && mix(colorWhite.x, testInputs.x, 0.0) == 1.0) && mix(colorWhite.xy, testInputs.xy, vec2(0.0, 0.5)) == vec2(1.0, 0.5)) && mix(colorWhite.xyz, testInputs.xyz, vec3(0.0, 0.5, 0.0)) == vec3(1.0, 0.5, 1.0)) && mix(colorWhite, testInputs, vec4(0.0, 0.5, 0.0, 1.0)) == vec4(1.0, 0.5, 1.0, 2.25) ? colorGreen : colorRed;
}
