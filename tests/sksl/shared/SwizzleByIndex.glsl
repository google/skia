
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorBlack;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 _0_v = testInputs;
    ivec4 _1_i = ivec4(colorBlack);
    float _2_x = _0_v[_1_i.x];
    float _3_y = _0_v[_1_i.y];
    float _4_z = _0_v[_1_i.z];
    float _5_w = _0_v[_1_i.w];
    return vec4(_2_x, _3_y, _4_z, _5_w) == vec4(-1.25, -1.25, -1.25, 0.0) ? colorGreen : colorRed;
}
