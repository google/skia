
out vec4 sk_FragColor;
void main() {
    struct Test {
        layout (offset = 0) int x;
        layout (offset = 4) int y;
        int z;
    } t;
    t.x = 0;
    sk_FragColor.x = float(t.x);
}
