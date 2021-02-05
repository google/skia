
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 _1_v = testInputs;
    float _2_x = _1_v.x;
    float _3_y = _1_v.y;
    float _4_z = _1_v.z;
    float _5_w = _1_v.w;
    vec4 a = vec4(_2_x, _3_y, _4_z, _5_w);

    return a == vec4(-1.25, 0.0, 0.75, 2.25) ? colorGreen : colorRed;
}
