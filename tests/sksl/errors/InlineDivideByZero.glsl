
uniform float unknownInput;
void main() {
    int _0_getZero;
    int inlineTest = 0 / 0;

    int _1_getZero;
    inlineTest = (ivec4(0) / 0).x;

    int _2_getZero;
    inlineTest = int(unknownInput) / 0;

}
