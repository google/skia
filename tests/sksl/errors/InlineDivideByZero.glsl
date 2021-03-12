
uniform float unknownInput;
void main() {
    int inlineTest = 0 / 0;
    inlineTest = (ivec4(0) / 0).x;
    inlineTest = int(unknownInput) / 0;
}
