
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
float foo(vec2 v) {
    return v.x * v.y;
}
void bar(inout float x) {
    float y[2];
    y[0] = x;
    y[1] = x * 2.0;
    x = foo(vec2(y[0], y[1]));
}
vec4 main() {
    float x = 10.0;
    bar(x);
    return x == 200.0 ? colorGreen : colorRed;
}
