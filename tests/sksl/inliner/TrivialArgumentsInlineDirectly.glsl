
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform float unknownInput;
uniform mat2 testMatrix2x2;
struct S {
    vec4 ah4[1];
    float ah[1];
    vec4 h4;
    float h;
};
struct S4 {
    float a;
    float b;
    float c;
    float d;
};
struct S5 {
    float a;
    float b;
    float c;
    float d;
    float e;
};
vec4 funcb_h4b(bool b) {
    return vec4(float(b), float(b), float(b), float(!b));
}
vec4 func1_h4h(float h) {
    return vec4(h) * vec4(h);
}
vec4 func2_h4h2(vec2 h2) {
    return h2.xyxy * h2.yxyx;
}
vec4 func3_h4h3(vec3 h3) {
    return h3.xyzx * h3.xyzx;
}
vec4 func4_h4h4(vec4 h4) {
    return h4 * h4;
}
mat2 func2x2_h22h22(mat2 m) {
    return m * m[0].x;
}
vec4 funcS4_h4S(S4 s) {
    return vec4(s.a, s.b, s.c, 1.0) * s.d;
}
vec4 funcS5_h4S(S5 s) {
    return vec4(s.a, s.b, s.c, s.d) * s.e;
}
vec4 main() {
    S s;
    s.ah4[0] = vec4(unknownInput);
    s.ah[0] = unknownInput;
    s.h4 = vec4(unknownInput);
    s.h = unknownInput;
    S as[1];
    as[0].ah4[0] = vec4(unknownInput);
    bool b = bool(unknownInput);
    int i = int(unknownInput);
    ivec4 i4 = ivec4(i);
    vec4 var;
    mat2 mat;
    var = func1_h4h(s.h);
    var = funcb_h4b(b);
    var = func2_h4h2(s.ah4[0].yw);
    var = func2_h4h2(as[0].ah4[0].xy);
    var = func3_h4h3(s.h4.zzz);
    var = func3_h4h3(colorGreen.xyz);
    var = func3_h4h3(vec3(s.h));
    var = func4_h4h4(vec4(s.h));
    var = func4_h4h4(s.ah4[0].xxxy);
    var = func4_h4h4(colorGreen);
    var = func4_h4h4(vec4(1.0, 2.0, 3.0, 4.0));
    var = func1_h4h(float(i));
    var = func4_h4h4(vec4(i4));
    var = funcS4_h4S(S4(1.0, 2.0, 3.0, 4.0));
    mat = func2x2_h22h22(mat2(unknownInput));
    var = func4_h4h4(vec4(testMatrix2x2));
    mat = func2x2_h22h22(mat2(colorGreen.xy, colorGreen.zw));
    mat = func2x2_h22h22(mat2(mat3(unknownInput)));
    var = func4_h4h4(vec4(1.0, 2.0, 3.0, unknownInput));
    var = funcS5_h4S(S5(1.0, 2.0, 3.0, 4.0, 5.0));
    var = func1_h4h(-s.h);
    var = funcb_h4b(!b);
    var = func3_h4h3(s.h4.yyy + s.h4.zzz);
    var = func4_h4h4(vec4(s.h4.y, 0.0, 0.0, 1.0));
    return colorGreen;
}
