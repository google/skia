
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool checkIntrinsicAsFunctionArg_bf3i3(vec3 f3, ivec3 e3) {
    return f3 == vec3(0.75) && e3 == ivec3(3);
}
vec4 main() {
    vec4 value = colorGreen.yyyy * 6.0;
    ivec4 _0_exp;
    vec4 result;
    bvec4 ok;
    result.x = frexp(value.x, _0_exp.x);
    ok.x = result.x == 0.75 && _0_exp.x == 3;
    result.xy = frexp(value.xy, _0_exp.xy);
    ok.y = result.y == 0.75 && _0_exp.y == 3;
    result.xyz = frexp(value.xyz, _0_exp.xyz);
    ok.z = result.z == 0.75 && _0_exp.z == 3;
    result = frexp(value, _0_exp);
    ok.w = result.w == 0.75 && _0_exp.w == 3;
    bool funcOk = checkIntrinsicAsFunctionArg_bf3i3(frexp(value.wzy, _0_exp.zxw).yxz, _0_exp.yxz);
    return all(ok) && funcOk ? colorGreen : colorRed;
}
