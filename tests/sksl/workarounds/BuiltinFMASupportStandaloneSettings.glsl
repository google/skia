
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float testArray[5];
vec4 main() {
    float one = testArray[0];
    float two = testArray[1];
    float three = testArray[2];
    float four = testArray[3];
    float five = testArray[4];
    return fma(one, two, three) == 5.0 && fma(three, four, five) == 17.0 ? colorGreen : colorRed;
}
