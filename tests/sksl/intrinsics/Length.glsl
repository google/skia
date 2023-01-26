
out vec4 sk_FragColor;
uniform vec4 testMatrix2x2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 inputVal = testMatrix2x2 + vec4(2.0, -2.0, 1.0, 8.0);
    vec4 expected = vec4(3.0, 3.0, 5.0, 13.0);
    const float allowedDelta = 0.05;
    return ((((((abs(length(inputVal.x) - expected.x) < allowedDelta && abs(length(inputVal.xy) - expected.y) < allowedDelta) && abs(length(inputVal.xyz) - expected.z) < allowedDelta) && abs(length(inputVal) - expected.w) < allowedDelta) && abs(3.0 - expected.x) < allowedDelta) && abs(3.0 - expected.y) < allowedDelta) && abs(5.0 - expected.z) < allowedDelta) && abs(13.0 - expected.w) < allowedDelta ? colorGreen : colorRed;
}
