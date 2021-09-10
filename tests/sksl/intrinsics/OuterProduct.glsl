
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform vec4 testInputs;
vec4 main() {
    const vec2 c12 = vec2(1.0, 2.0);
    return ((((outerProduct(testMatrix2x2[0], testMatrix2x2[1]) == mat2(3.0, 6.0, 4.0, 8.0) && outerProduct(testMatrix3x3[0], testMatrix3x3[1]) == mat3(4.0, 8.0, 12.0, 5.0, 10.0, 15.0, 6.0, 12.0, 18.0)) && outerProduct(testMatrix2x2[0], testMatrix3x3[1]) == mat3x2(4.0, 8.0, 5.0, 10.0, 6.0, 12.0)) && outerProduct(testInputs, vec4(1.0, 0.0, 0.0, 2.0)) == mat4(-1.25, 0.0, 0.75, 2.25, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -2.5, 0.0, 1.5, 4.5)) && outerProduct(testInputs, c12) == mat2x4(-1.25, 0.0, 0.75, 2.25, -2.5, 0.0, 1.5, 4.5)) && outerProduct(c12, testInputs) == mat4x2(-1.25, -2.5, 0.0, 0.0, 0.75, 1.5, 2.25, 4.5) ? colorGreen : colorRed;
}
