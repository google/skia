
out vec4 sk_FragColor;
float foo(float v[2]) {
    return v[0] * v[1];
}
void bar(inout float x) {
    float y[2];

    y[0] = x;
    y[1] = x * 2.0;
    foo(y);
}
void main() {
    float x = 10.0;
    bar(x);
    sk_FragColor = vec4(x);
}
