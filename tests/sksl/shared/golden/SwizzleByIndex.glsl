
out vec4 sk_FragColor;
void main() {
    ivec4 _0_i = ivec4(int(sqrt(1.0)));
    vec4 _1_v = vec4(sqrt(1.0));
    float _2_x = _1_v[_0_i.x];
    float _3_y = _1_v[_0_i.y];
    float _4_z = _1_v[_0_i.z];
    float _5_w = _1_v[_0_i.w];
    sk_FragColor = vec4(_2_x, _3_y, _4_z, _5_w);

    vec4 _6_v = vec4(sqrt(1.0));
    float _7_x = _6_v.x;
    float _8_y = _6_v.y;
    float _9_z = _6_v.z;
    float _10_w = _6_v.w;
    sk_FragColor = vec4(_7_x, _8_y, _9_z, _10_w);

    sk_FragColor = vec4(2.0, 2.0, 2.0, 2.0);

}
