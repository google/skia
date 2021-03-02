
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

    vec4 _5_v = vec4(0.0, 1.0, 2.0, 3.0);
    float _6_x = _5_v.x;
    float _7_y = _5_v.y;
    float _8_z = _5_v.z;
    float _9_w = _5_v.w;
    vec4 b = vec4(_6_x, _7_y, _8_z, _9_w);

    return a == vec4(-1.25, 0.0, 0.75, 2.25) && b == vec4(0.0, 1.0, 2.0, 3.0) ? colorGreen : colorRed;
}
