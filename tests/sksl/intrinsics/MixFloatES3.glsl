
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorBlack;
uniform vec4 colorWhite;
uniform vec4 testInputs;
vec4 main() {
    bvec4 FTFT = bvec4(colorGreen);
    bvec4 TFTF = FTFT.wzyx;
    return ((((((mix(colorBlack.x, colorWhite.x, FTFT.x) == colorBlack.x && mix(colorBlack.xy, colorWhite.xy, FTFT.xy) == vec2(colorBlack.x, 1.0)) && mix(colorBlack.xyz, colorWhite.xyz, FTFT.xyz) == vec3(colorBlack.x, 1.0, colorBlack.z)) && mix(colorBlack, colorWhite, FTFT) == vec4(colorBlack.x, 1.0, colorBlack.z, 1.0)) && mix(colorWhite.x, testInputs.x, TFTF.x) == testInputs.x) && mix(colorWhite.xy, testInputs.xy, TFTF.xy) == vec2(testInputs.x, 1.0)) && mix(colorWhite.xyz, testInputs.xyz, TFTF.xyz) == vec3(testInputs.x, 1.0, testInputs.z)) && mix(colorWhite, testInputs, TFTF) == vec4(testInputs.x, 1.0, testInputs.z, 1.0) ? colorGreen : colorRed;
}
