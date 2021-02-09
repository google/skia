
out vec4 sk_FragColor;
struct A {
    int x;
    int y;
};
A a1;
A a2;
A a3;
struct B {
    float x;
    float[2] y;
    layout (binding = 1) A z;
};
B b1;
B b2;
B b3;
void main() {
    a1.x = 0;
    b1.x = 0.0;
    sk_FragColor.x = float(a1.x) + b1.x;
}
