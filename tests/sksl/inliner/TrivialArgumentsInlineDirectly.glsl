
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
    vec3 _0_h3 = vec3(s.h);
    var = _0_h3.xyzx * _0_h3.xyzx;
    vec4 _1_h4 = vec4(s.h);
    var = _1_h4 * _1_h4;
    var = s.ah4[0].xxxy * s.ah4[0].xxxy;
    var = colorGreen * colorGreen;
    float _2_h = -s.h;
    var = vec4(_2_h) * vec4(_2_h);
    bool _3_b = !b;
    var = vec4(float(_3_b), float(_3_b), float(_3_b), float(!_3_b));
    vec3 _4_h3 = s.h4.yyy + s.h4.zzz;
    var = _4_h3.xyzx * _4_h3.xyzx;
    vec4 _5_h4 = vec4(s.h4.y, 0.0, 0.0, 1.0);
    var = _5_h4 * _5_h4;
    return colorGreen;
}
