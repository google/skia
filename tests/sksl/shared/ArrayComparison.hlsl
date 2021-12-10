struct S
{
    int x;
    int y;
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float _35[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float f1[4] = _35;
    float _37[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float f2[4] = _37;
    float _40[4] = { 1.0f, 2.0f, 3.0f, -4.0f };
    float f3[4] = _40;
    int3 _52[2] = { int3(1, 2, 3), int3(4, 5, 6) };
    int3 v1[2] = _52;
    int3 _54[2] = { int3(1, 2, 3), int3(4, 5, 6) };
    int3 v2[2] = _54;
    int3 _58[2] = { int3(1, 2, 3), int3(4, 5, -6) };
    int3 v3[2] = _58;
    float2x2 _74[3] = { float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f)), float2x2(float2(3.0f, 4.0f), float2(5.0f, 6.0f)) };
    float2x2 m1[3] = _74;
    float2x2 _85[3] = { float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f)), float2x2(float2(3.0f, 4.0f), float2(5.0f, 6.0f)) };
    float2x2 m2[3] = _85;
    float2x2 _96[3] = { float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f)), float2x2(float2(2.0f, 3.0f), float2(4.0f, 5.0f)), float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f)) };
    float2x2 m3[3] = _96;
    S _101 = { 1, 2 };
    S _102 = { 3, 4 };
    S _103 = { 5, 6 };
    S _104[3] = { _101, _102, _103 };
    S s1[3] = _104;
    S _106 = { 1, 2 };
    S _108 = { 0, 0 };
    S _109 = { 5, 6 };
    S _110[3] = { _106, _108, _109 };
    S s2[3] = _110;
    S _112 = { 1, 2 };
    S _113 = { 3, 4 };
    S _114 = { 5, 6 };
    S _115[3] = { _112, _113, _114 };
    S s3[3] = _115;
    bool _153 = false;
    if ((f1[3] == f2[3]) && ((f1[2] == f2[2]) && ((f1[1] == f2[1]) && (f1[0] == f2[0]))))
    {
        _153 = (f1[3] != f3[3]) || ((f1[2] != f3[2]) || ((f1[1] != f3[1]) || (f1[0] != f3[0])));
    }
    else
    {
        _153 = false;
    }
    bool _168 = false;
    if (_153)
    {
        _168 = all(bool3(v1[1].x == v2[1].x, v1[1].y == v2[1].y, v1[1].z == v2[1].z)) && all(bool3(v1[0].x == v2[0].x, v1[0].y == v2[0].y, v1[0].z == v2[0].z));
    }
    else
    {
        _168 = false;
    }
    bool _182 = false;
    if (_168)
    {
        _182 = any(bool3(v1[1].x != v3[1].x, v1[1].y != v3[1].y, v1[1].z != v3[1].z)) || any(bool3(v1[0].x != v3[0].x, v1[0].y != v3[0].y, v1[0].z != v3[0].z));
    }
    else
    {
        _182 = false;
    }
    bool _223 = false;
    if (_182)
    {
        _223 = (all(bool2(m1[2][0].x == m2[2][0].x, m1[2][0].y == m2[2][0].y)) && all(bool2(m1[2][1].x == m2[2][1].x, m1[2][1].y == m2[2][1].y))) && ((all(bool2(m1[1][0].x == m2[1][0].x, m1[1][0].y == m2[1][0].y)) && all(bool2(m1[1][1].x == m2[1][1].x, m1[1][1].y == m2[1][1].y))) && (all(bool2(m1[0][0].x == m2[0][0].x, m1[0][0].y == m2[0][0].y)) && all(bool2(m1[0][1].x == m2[0][1].x, m1[0][1].y == m2[0][1].y))));
    }
    else
    {
        _223 = false;
    }
    bool _263 = false;
    if (_223)
    {
        _263 = (any(bool2(m1[2][0].x != m3[2][0].x, m1[2][0].y != m3[2][0].y)) || any(bool2(m1[2][1].x != m3[2][1].x, m1[2][1].y != m3[2][1].y))) || ((any(bool2(m1[1][0].x != m3[1][0].x, m1[1][0].y != m3[1][0].y)) || any(bool2(m1[1][1].x != m3[1][1].x, m1[1][1].y != m3[1][1].y))) || (any(bool2(m1[0][0].x != m3[0][0].x, m1[0][0].y != m3[0][0].y)) || any(bool2(m1[0][1].x != m3[0][1].x, m1[0][1].y != m3[0][1].y))));
    }
    else
    {
        _263 = false;
    }
    bool _297 = false;
    if (_263)
    {
        _297 = ((s1[2].y != s2[2].y) || (s1[2].x != s2[2].x)) || (((s1[1].y != s2[1].y) || (s1[1].x != s2[1].x)) || ((s1[0].y != s2[0].y) || (s1[0].x != s2[0].x)));
    }
    else
    {
        _297 = false;
    }
    bool _331 = false;
    if (_297)
    {
        _331 = ((s3[2].y == s1[2].y) && (s3[2].x == s1[2].x)) && (((s3[1].y == s1[1].y) && (s3[1].x == s1[1].x)) && ((s3[0].y == s1[0].y) && (s3[0].x == s1[0].x)));
    }
    else
    {
        _331 = false;
    }
    float4 _332 = 0.0f.xxxx;
    if (_331)
    {
        _332 = _10_colorGreen;
    }
    else
    {
        _332 = _10_colorRed;
    }
    return _332;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
