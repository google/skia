
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
const int SEVEN = 7;
const int TEN = 10;
const mat4 MATRIXFIVE = mat4(5.0);
bool verify_const_globals_biih44(int seven, int ten, mat4 matrixFive) {
    return (seven == 7 && ten == 10) && matrixFive == mat4(5.0);
}
vec4 main() {
    return verify_const_globals_biih44(SEVEN, TEN, MATRIXFIVE) ? colorGreen : colorRed;
}
