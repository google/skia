
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
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
    return all(ok) ? colorGreen : colorRed;
}
