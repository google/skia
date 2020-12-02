
out vec4 sk_FragColor;
float arr(float v[3][2]) {
    return v[0][0] * v[1][2];
}
float foo(float v[2]) {
    return v[0] * v[1];
}
void bar(inout float x) {
    float y[2];
    float z;

    y[0] = x;
    y[1] = x * 2.0;
    z = foo(y);
    float a[2][3];
    a[0][0] = 123.0;
    a[1][2] = 456.0;
    x = z + arr(a);
}
void main() {
    float x = 10.0;
    bar(x);
    sk_FragColor = vec4(x);
}
