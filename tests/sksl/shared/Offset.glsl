
out vec4 sk_FragColor;
struct Test {
    layout (offset = 0) int x;
    layout (offset = 4) int y;
    int z;
};
void main() {
    Test t;
    t.x = 0;
    sk_FragColor.x = float(t.x);
}
