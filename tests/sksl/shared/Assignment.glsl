
out vec4 sk_FragColor;
uniform vec4 colorGreen;
struct S {
    float f;
    float af[5];
    vec4 h4;
    vec4 ah4[5];
};
vec4 globalVar;
S globalStruct;
void keepAlive_vh(inout float h) {
}
void keepAlive_vf(inout float f) {
}
void keepAlive_vi(inout int i) {
}
void assignToFunctionParameter_vif(int x, inout float y) {
    x = 1;
    y = 1.0;
}
vec4 main() {
    int i = 0;
    ivec4 i4 = ivec4(1, 2, 3, 4);
    mat3 f3x3 = mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    vec4 x;
    x.w = 0.0;
    x.yx = vec2(0.0);
    int ai[1];
    ai[0] = 0;
    ivec4 ai4[1];
    ai4[0] = ivec4(1, 2, 3, 4);
    mat3 ah3x3[1];
    ah3x3[0] = mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    vec4 af4[1];
    af4[0].x = 0.0;
    af4[0].ywxz = vec4(1.0);
    S s;
    s.f = 0.0;
    s.af[1] = 0.0;
    s.h4.zxy = vec3(9.0);
    s.ah4[2].yw = vec2(5.0);
    globalVar = vec4(0.0);
    globalStruct.f = 0.0;
    assignToFunctionParameter_vif(0, f3x3[0].x);
    float l;
    l = 0.0;
    ai[0] += ai4[0].x;
    s.f = 1.0;
    s.af[0] = 2.0;
    s.h4 = vec4(1.0);
    s.ah4[0] = vec4(2.0);
    float repeat;
    repeat = (repeat = 1.0);
    keepAlive_vf(af4[0].x);
    keepAlive_vh(ah3x3[0][0].x);
    keepAlive_vi(i);
    keepAlive_vi(i4.y);
    keepAlive_vi(ai[0]);
    keepAlive_vi(ai4[0].x);
    keepAlive_vh(x.y);
    keepAlive_vf(s.f);
    keepAlive_vh(l);
    keepAlive_vf(f3x3[0].x);
    keepAlive_vf(repeat);
    return colorGreen;
}
