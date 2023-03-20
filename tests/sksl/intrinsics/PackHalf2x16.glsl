
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 testInputs;
vec4 main() {
    uint xy = packHalf2x16(testInputs.xy);
    uint zw = packHalf2x16(testInputs.zw);
    return unpackHalf2x16(xy) == vec2(-1.25, 0.0) && unpackHalf2x16(zw) == vec2(0.75, 2.25) ? colorGreen : colorRed;
}
