
float foo_ff(float v) {
    return v;
}
void bar_v() {
    float y = 0.0;
    foo_ff(y);
}
void main() {
    bar_v();
}
