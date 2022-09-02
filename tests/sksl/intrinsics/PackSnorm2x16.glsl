
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 testInputs;
vec4 main() {
    uint xy = packSnorm2x16(testInputs.xy);
    uint zw = packSnorm2x16(testInputs.zw);
    const vec2 tolerance = vec2(0.015625);
    return all(lessThan(abs(unpackSnorm2x16(xy) - vec2(-1.0, 0.0)), tolerance)) && all(lessThan(abs(unpackSnorm2x16(zw) - vec2(0.75, 1.0)), tolerance)) ? colorGreen : colorRed;
}
