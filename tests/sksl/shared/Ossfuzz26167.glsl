
float foo(float v) {
    return v;
}
void bar() {
    float y = 0.0;
    float z = foo(y);

}
void main() {
    bar();
}
