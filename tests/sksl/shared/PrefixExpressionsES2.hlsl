cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool ok = true;
    int i = 5;
    int _35 = 5 + 1;
    i = _35;
    bool _41 = false;
    if (true)
    {
        _41 = _35 == 6;
    }
    else
    {
        _41 = false;
    }
    ok = _41;
    bool _47 = false;
    if (_41)
    {
        int _44 = _35 + 1;
        i = _44;
        _47 = _44 == 7;
    }
    else
    {
        _47 = false;
    }
    ok = _47;
    bool _53 = false;
    if (_47)
    {
        int _50 = i;
        int _51 = _50 - 1;
        i = _51;
        _53 = _51 == 6;
    }
    else
    {
        _53 = false;
    }
    ok = _53;
    int _54 = i;
    int _55 = _54 - 1;
    i = _55;
    bool _59 = false;
    if (_53)
    {
        _59 = _55 == 5;
    }
    else
    {
        _59 = false;
    }
    ok = _59;
    float f = 0.5f;
    float _64 = 0.5f + 1.0f;
    f = _64;
    bool _69 = false;
    if (_59)
    {
        _69 = _64 == 1.5f;
    }
    else
    {
        _69 = false;
    }
    ok = _69;
    bool _75 = false;
    if (_69)
    {
        float _72 = _64 + 1.0f;
        f = _72;
        _75 = _72 == 2.5f;
    }
    else
    {
        _75 = false;
    }
    ok = _75;
    bool _81 = false;
    if (_75)
    {
        float _78 = f;
        float _79 = _78 - 1.0f;
        f = _79;
        _81 = _79 == 1.5f;
    }
    else
    {
        _81 = false;
    }
    ok = _81;
    float _82 = f;
    float _83 = _82 - 1.0f;
    f = _83;
    bool _87 = false;
    if (_81)
    {
        _87 = _83 == 0.5f;
    }
    else
    {
        _87 = false;
    }
    ok = _87;
    float2 f2 = 0.5f.xx;
    f2.x += 1.0f;
    bool _100 = false;
    if (ok)
    {
        _100 = f2.x == 1.5f;
    }
    else
    {
        _100 = false;
    }
    ok = _100;
    bool _107 = false;
    if (_100)
    {
        float _104 = f2.x;
        float _105 = _104 + 1.0f;
        f2.x = _105;
        _107 = _105 == 2.5f;
    }
    else
    {
        _107 = false;
    }
    ok = _107;
    bool _114 = false;
    if (_107)
    {
        float _111 = f2.x;
        float _112 = _111 - 1.0f;
        f2.x = _112;
        _114 = _112 == 1.5f;
    }
    else
    {
        _114 = false;
    }
    ok = _114;
    f2.x -= 1.0f;
    bool _124 = false;
    if (ok)
    {
        _124 = f2.x == 0.5f;
    }
    else
    {
        _124 = false;
    }
    ok = _124;
    bool _133 = false;
    if (_124)
    {
        _133 = !(_10_colorGreen.x == 1.0f);
    }
    else
    {
        _133 = false;
    }
    ok = _133;
    bool _142 = false;
    if (_133)
    {
        _142 = (-1.0f) == (-_10_colorGreen.y);
    }
    else
    {
        _142 = false;
    }
    ok = _142;
    bool _152 = false;
    if (_142)
    {
        float4 _148 = -_10_colorGreen;
        _152 = all(bool4(float4(0.0f, -1.0f, 0.0f, -1.0f).x == _148.x, float4(0.0f, -1.0f, 0.0f, -1.0f).y == _148.y, float4(0.0f, -1.0f, 0.0f, -1.0f).z == _148.z, float4(0.0f, -1.0f, 0.0f, -1.0f).w == _148.w));
    }
    else
    {
        _152 = false;
    }
    ok = _152;
    bool _176 = false;
    if (_152)
    {
        float2 _166 = -_10_testMatrix2x2[0];
        float2 _168 = -_10_testMatrix2x2[1];
        _176 = all(bool2(float2(-1.0f, -2.0f).x == _166.x, float2(-1.0f, -2.0f).y == _166.y)) && all(bool2(float2(-3.0f, -4.0f).x == _168.x, float2(-3.0f, -4.0f).y == _168.y));
    }
    else
    {
        _176 = false;
    }
    ok = _176;
    int2 _183 = int2(i, -i);
    int2 iv = _183;
    bool _190 = false;
    if (_176)
    {
        _190 = (-i) == (-5);
    }
    else
    {
        _190 = false;
    }
    ok = _190;
    bool _197 = false;
    if (_190)
    {
        int2 _193 = -_183;
        _197 = all(bool2(_193.x == int2(-5, 5).x, _193.y == int2(-5, 5).y));
    }
    else
    {
        _197 = false;
    }
    ok = _197;
    float4 _198 = 0.0f.xxxx;
    if (_197)
    {
        _198 = _10_colorGreen;
    }
    else
    {
        _198 = _10_colorRed;
    }
    return _198;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
