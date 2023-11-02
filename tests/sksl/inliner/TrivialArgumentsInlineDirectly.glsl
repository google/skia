
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
void keepAlive_vh(inout float f) {
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
    var = vec4(s.h) * vec4(s.h);
    var = vec4(float(b), float(b), float(b), float(!b));
    var = s.ah4[0].ywyw * s.ah4[0].wywy;
    var = as[0].ah4[0].xyxy * as[0].ah4[0].yxyx;
    var = s.h4.zzzz * s.h4.zzzz;
    var = colorGreen.xyzx * colorGreen.xyzx;
    var = vec4(s.h) * vec4(s.h);
    var = vec4(s.h) * vec4(s.h);
    var = s.ah4[0].xxxy * s.ah4[0].xxxy;
    var = colorGreen * colorGreen;
    var = vec4(1.0, 4.0, 9.0, 16.0);
    var = vec4(float(i)) * vec4(float(i));
    var = vec4(i4) * vec4(i4);
    var = vec4(4.0, 8.0, 12.0, 4.0);
    mat = mat2(unknownInput) * mat2(unknownInput)[0].x;
    vec4 _0_h4 = vec4(testMatrix2x2);
    var = _0_h4 * _0_h4;
    mat2 _1_m = mat2(colorGreen.xy, colorGreen.zw);
    mat = _1_m * _1_m[0].x;
    mat2 _2_m = mat2(mat3(unknownInput));
    mat = _2_m * _2_m[0].x;
    vec4 _3_h4 = vec4(1.0, 2.0, 3.0, unknownInput);
    var = _3_h4 * _3_h4;
    S5 _4_s = S5(1.0, 2.0, 3.0, 4.0, 5.0);
    var = vec4(_4_s.a, _4_s.b, _4_s.c, _4_s.d) * _4_s.e;
    var = vec4(-s.h) * vec4(-s.h);
    var = vec4(float(!b), float(!b), float(!b), float(b));
    vec3 _5_h3 = s.h4.yyy + s.h4.zzz;
    var = _5_h3.xyzx * _5_h3.xyzx;
    vec4 _6_h4 = vec4(s.h4.y, 0.0, 0.0, 1.0);
    var = _6_h4 * _6_h4;
    keepAlive_vh(var.x);
    keepAlive_vh(mat[0].x);
    return colorGreen;
}
