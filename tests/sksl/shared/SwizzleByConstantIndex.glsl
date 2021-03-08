
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

    float _5_x = testInputs.x;
    float _6_y = testInputs.y;
    float _7_z = testInputs.z;
    float _8_w = testInputs.w;
    vec4 b = vec4(_5_x, _6_y, _7_z, _8_w);

    return a == vec4(-1.25, 0.0, 0.75, 2.25) && b == vec4(-1.25, 0.0, 0.75, 2.25) ? colorGreen : colorRed;
}
