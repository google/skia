
out vec4 sk_FragColor;
void main() {
    {
        vec4 _0_v = vec4(sqrt(1.0));
        float _1_x = _0_v.x;
        float _2_y = _0_v.y;
        float _3_z = _0_v.z;
        float _4_w = _0_v.w;
        sk_FragColor = vec4(_1_x, _2_y, _3_z, _4_w);
    }

    {
        sk_FragColor = vec4(2.0, 2.0, 2.0, 2.0);
    }

}
