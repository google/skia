
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(0.0);
    return ((((((dFdy(testInputs.x)) == expected.x && (dFdy(testInputs.xy)) == expected.xy) && (dFdy(testInputs.xyz)) == expected.xyz) && (dFdy(testInputs)) == expected) && sign((dFdy(coords.xx))) == vec2(0.0)) && sign((dFdy(coords.yy))) == vec2(1.0)) && sign((dFdy(coords))) == vec2(0.0, 1.0) ? colorGreen : colorRed;
}
