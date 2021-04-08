
out vec4 sk_FragColor;
struct A {
    int x;
    float y;
};
A a1;
A a4 = A(1, 2.0);
struct B {
    float x;
    float y[2];
    layout (binding = 1) A z;
};
B b1;
B b4 = B(1.0, float[2](2.0, 3.0), A(4, 5.0));
struct C {
    A i;
    A j;
};
void main() {
    a1.x = 0;
    b1.x = 0.0;
    sk_FragColor.x = (((float(a1.x) + b1.x) + a4.y) + b4.x) + A(1, 2.0).y;
    C c1;
    C c2;
    C c3;
    c1 = C(A(1, 2.0), A(3, 4.0));
    c2 = C(A(1, -2.0), A(-3, 4.0));
    c3 = C(A(1, 2.0), A(3, 4.0));
    sk_FragColor.y = float(c1 == c3);
    sk_FragColor.z = float(c2 != c3);
    sk_FragColor.w = float(c1.i != c2.j);
}
