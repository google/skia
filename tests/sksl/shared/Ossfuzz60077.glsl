
out vec4 sk_FragColor;
void d_vi(int _skAnonymousParam0) {
    int b = 4;
}
void c_vi(int i) {
    d_vi(i);
}
void b_vi(int i) {
    c_vi(i);
}
void a_vi(int i) {
    b_vi(i);
    b_vi(i);
}
vec4 main() {
    int i;
    a_vi(i);
    return vec4(0.0);
}
