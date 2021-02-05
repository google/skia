
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorBlack;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 _1_v = testInputs;
    ivec4 _2_i = ivec4(colorBlack);
    float _3_x = _1_v[_2_i.x];
    float _4_y = _1_v[_2_i.y];
    float _5_z = _1_v[_2_i.z];
    float _6_w = _1_v[_2_i.w];
    return vec4(_3_x, _4_y, _5_z, _6_w) == vec4(-1.25, -1.25, -1.25, 0.0) ? colorGreen : colorRed;

}
