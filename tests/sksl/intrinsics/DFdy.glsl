
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(0.0);
    return (((((dFdx(testInputs.x) == expected.x && dFdx(testInputs.xy) == expected.xy) && dFdx(testInputs.xyz) == expected.xyz) && dFdx(testInputs) == expected) && sign(u_skRTFlip.y * dFdy(coords.xx)) == vec2(0.0, 0.0)) && sign(u_skRTFlip.y * dFdy(coords.yy)) == vec2(1.0, 1.0)) && sign(u_skRTFlip.y * dFdy(coords)) == vec2(0.0, 1.0) ? colorGreen : colorRed;
}
