
out vec4 sk_FragColor;
in float val;
uniform int ui;
uniform vec4 uh4;
uniform bool b;
struct S {
    vec4 ah4[1];
    float ah[1];
    vec4 h4;
    float h;
};
void funcb(bool b, out vec4 outColor) {
    outColor = b ? outColor.xxxx : outColor.yyyy;
}
void func1(float h, out vec4 outColor) {
    outColor = vec4(h);
}
void func2(vec2 h2, out vec4 outColor) {
    outColor = h2.xyxy;
}
void func3(vec3 h3, out vec4 outColor) {
    outColor = h3.xyzx;
}
void func4(vec4 h4, out vec4 outColor) {
    outColor = h4;
}
void main() {
    S s;
    s.ah4[0] = vec4(val);
    s.ah[0] = val;
    s.h4 = vec4(val);
    s.h = val;
    S as[1];
    as[0].ah4[0] = vec4(val);
    funcb(true, sk_FragColor);
    func1(s.h, sk_FragColor);
    funcb(b, sk_FragColor);
    func2(s.ah4[0].yw, sk_FragColor);
    func2(as[0].ah4[0].xy, sk_FragColor);
    func3(s.h4.zzz, sk_FragColor);
    func3(uh4.xyz, sk_FragColor);
    func3(vec3(s.h), sk_FragColor);
    func4(vec4(s.h), sk_FragColor);
    func4(s.ah4[0].xxxy, sk_FragColor);
    func4(uh4, sk_FragColor);
    funcb(false, sk_FragColor);
    func1(-s.h, sk_FragColor);
    funcb(!b, sk_FragColor);
    func2(s.ah4[ui].yw, sk_FragColor);
    func3(s.h4.yyy + s.h4.zzz, sk_FragColor);
    func4(vec4(s.h4.y, 0.0, 0.0, 1.0), sk_FragColor);
}
