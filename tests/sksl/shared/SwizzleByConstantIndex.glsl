
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 _0_v = testInputs;
    float _1_x = _0_v.x;
    float _2_y = _0_v.y;
    float _3_z = _0_v.z;
    float _4_w = _0_v.w;
    vec4 a = vec4(_1_x, _2_y, _3_z, _4_w);
    float _9_x = testInputs.x;
    float _10_y = testInputs.y;
    float _11_z = testInputs.z;
    float _12_w = testInputs.w;
    vec4 b = vec4(_9_x, _10_y, _11_z, _12_w);
    vec4 _13_v = vec4(0.0, 1.0, 2.0, 3.0);
    float _14_x = _13_v.x;
    float _15_y = _13_v.y;
    float _16_z = _13_v.z;
    float _17_w = _13_v.w;
    vec4 c = vec4(_14_x, _15_y, _16_z, _17_w);
    return (a == vec4(-1.25, 0.0, 0.75, 2.25) && b == vec4(-1.25, 0.0, 0.75, 2.25)) && c == vec4(0.0, 1.0, 2.0, 3.0) ? colorGreen : colorRed;
}
