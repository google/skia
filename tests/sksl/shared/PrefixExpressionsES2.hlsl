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
    bool _132 = false;
    if (_124)
    {
        _132 = _10_colorGreen.x != 1.0f;
    }
    else
    {
        _132 = false;
    }
    ok = _132;
    bool _141 = false;
    if (_132)
    {
        _141 = (-1.0f) == (-_10_colorGreen.y);
    }
    else
    {
        _141 = false;
    }
    ok = _141;
    bool _151 = false;
    if (_141)
    {
        float4 _147 = -_10_colorGreen;
        _151 = all(bool4(float4(0.0f, -1.0f, 0.0f, -1.0f).x == _147.x, float4(0.0f, -1.0f, 0.0f, -1.0f).y == _147.y, float4(0.0f, -1.0f, 0.0f, -1.0f).z == _147.z, float4(0.0f, -1.0f, 0.0f, -1.0f).w == _147.w));
    }
    else
    {
        _151 = false;
    }
    ok = _151;
    bool _175 = false;
    if (_151)
    {
        float2 _165 = -_10_testMatrix2x2[0];
        float2 _167 = -_10_testMatrix2x2[1];
        _175 = all(bool2(float2(-1.0f, -2.0f).x == _165.x, float2(-1.0f, -2.0f).y == _165.y)) && all(bool2(float2(-3.0f, -4.0f).x == _167.x, float2(-3.0f, -4.0f).y == _167.y));
    }
    else
    {
        _175 = false;
    }
    ok = _175;
    int2 _182 = int2(i, -i);
    int2 iv = _182;
    bool _189 = false;
    if (_175)
    {
        _189 = (-i) == (-5);
    }
    else
    {
        _189 = false;
    }
    ok = _189;
    bool _196 = false;
    if (_189)
    {
        int2 _192 = -_182;
        _196 = all(bool2(_192.x == int2(-5, 5).x, _192.y == int2(-5, 5).y));
    }
    else
    {
        _196 = false;
    }
    ok = _196;
    float4 _197 = 0.0f.xxxx;
    if (_196)
    {
        _197 = _10_colorGreen;
    }
    else
    {
        _197 = _10_colorRed;
    }
    return _197;
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
