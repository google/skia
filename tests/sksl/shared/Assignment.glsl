
out vec4 sk_FragColor;
struct S {
    float f;
    float[5] af;
    vec4 h4;
    vec4[5] ah4;
};
void main() {
    vec4 x;
    x.w = 0.0;
    x.yx = vec2(0.0);
    int ai[1];
    ai[0] = 0;
    ivec4 ai4[1];
    ai4[0] = ivec4(1, 2, 3, 4);
    mat2x4 ah2x4[1];
    ah2x4[0] = mat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    ai[0] = 0;
    ai[ai[0]] = 0;
    vec4 af4[1];
    af4[0].x = 0.0;
    af4[0].ywxz = vec4(1.0);
    S s;
    s.f = 0.0;
    s.af[1] = 0.0;
    s.h4.zxy = vec3(9.0);
    s.ah4[2].yw = vec2(5.0);
    s.f = 1.0;
    s.af[0] = 2.0;
    s.h4 = vec4(1.0);
    s.ah4[0] = vec4(2.0);
    sk_FragColor = vec4(0.0);
    sk_FragColor = vec4(ivec4(1, 2, 3, 4));
    sk_FragColor = mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0)[0].xxyz;
    sk_FragColor = x;
    sk_FragColor = vec4(float(ai[0]));
    sk_FragColor = vec4(ai4[0]);
    sk_FragColor = ah2x4[0][0];
    sk_FragColor = af4[0];
    sk_FragColor = vec4(0.0);
    sk_FragColor = vec4(s.f);
    sk_FragColor = vec4(s.af[1]);
    sk_FragColor = s.h4;
    sk_FragColor = s.ah4[0];
}
