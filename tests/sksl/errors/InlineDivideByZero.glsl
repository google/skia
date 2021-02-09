
uniform float unknownInput;
int getZero() {
    return 0;
}
void main() {
    int inlineTest = 0 / getZero();
    inlineTest = (ivec4(0) / getZero()).x;
    inlineTest = int(unknownInput) / getZero();
}
