
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform float unknownInput;
struct S {
    vec4 ah4[1];
    float ah[1];
    vec4 h4;
    float h;
};
vec4 main() {
    S s;
    s.ah4[0] = vec4(unknownInput);
    s.ah[0] = unknownInput;
    s.h4 = vec4(unknownInput);
    s.h = unknownInput;
    S as[1];
    as[0].ah4[0] = vec4(unknownInput);
    bool b = bool(unknownInput);
    vec4 var;
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
    float _0_h = -s.h;
    var = vec4(_0_h) * vec4(_0_h);
    bool _1_b = !b;
    var = vec4(float(_1_b), float(_1_b), float(_1_b), float(!_1_b));
    vec3 _2_h3 = s.h4.yyy + s.h4.zzz;
    var = _2_h3.xyzx * _2_h3.xyzx;
    vec4 _3_h4 = vec4(s.h4.y, 0.0, 0.0, 1.0);
    var = _3_h4 * _3_h4;
    return colorGreen;
}
