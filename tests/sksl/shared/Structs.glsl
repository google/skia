
out vec4 sk_FragColor;
struct A {
    int x;
    int y;
};
A a1;
struct B {
    float x;
    float y[2];
    A z;
};
B b1;
void main() {
    a1.x = 0;
    b1.x = 0.0;
    sk_FragColor.x = float(a1.x) + b1.x;
}
