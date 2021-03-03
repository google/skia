
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    float i;
    float j;
};
vec4 main() {
    vec4 x = vec4(3.0, 2.0, 1.0, 0.0);
    x.xyz = x.zyx;
    S s;
    s.i = 2.0;
    s.j = 2.0;
    s.i = s.j;
    s.j = s.i;
    float a[2];
    a[0] = 1.0;
    a[1] = 0.0;
    a[1] = a[0];
    return vec4(x.w, s.i / s.j, a[0] - a[1], a[0] * a[1]);
}
