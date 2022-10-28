
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    int a;
    int b;
    int c;
};
vec4 main() {
    const int _6_two = 2;
    int _8_flatten1 = _6_two;
    return _8_flatten1 == 2 ? colorGreen : colorRed;
}
