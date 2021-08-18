
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(0.0);
    return (((((dFdx(testInputs.x) == expected.x && dFdx(testInputs.xy) == expected.xy) && dFdx(testInputs.xyz) == expected.xyz) && dFdx(testInputs) == expected) && sign(dFdx(coords.xx)) == vec2(1.0, 1.0)) && sign(dFdx(coords.yy)) == vec2(0.0, 0.0)) && sign(dFdx(coords)) == vec2(1.0, 0.0) ? colorGreen : colorRed;
}
