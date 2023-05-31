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
    bool _97 = false;
    if (_87)
    {
        _97 = !(_10_colorGreen.x == 1.0f);
    }
    else
    {
        _97 = false;
    }
    ok = _97;
    bool _106 = false;
    if (_97)
    {
        _106 = (-1.0f) == (-_10_colorGreen.y);
    }
    else
    {
        _106 = false;
    }
    ok = _106;
    bool _116 = false;
    if (_106)
    {
        float4 _112 = -_10_colorGreen;
        _116 = all(bool4(float4(0.0f, -1.0f, 0.0f, -1.0f).x == _112.x, float4(0.0f, -1.0f, 0.0f, -1.0f).y == _112.y, float4(0.0f, -1.0f, 0.0f, -1.0f).z == _112.z, float4(0.0f, -1.0f, 0.0f, -1.0f).w == _112.w));
    }
    else
    {
        _116 = false;
    }
    ok = _116;
    bool _140 = false;
    if (_116)
    {
        float2 _130 = -_10_testMatrix2x2[0];
        float2 _132 = -_10_testMatrix2x2[1];
        _140 = all(bool2(float2(-1.0f, -2.0f).x == _130.x, float2(-1.0f, -2.0f).y == _130.y)) && all(bool2(float2(-3.0f, -4.0f).x == _132.x, float2(-3.0f, -4.0f).y == _132.y));
    }
    else
    {
        _140 = false;
    }
    ok = _140;
    int2 _145 = int2(_55, -_55);
    int2 iv = _145;
    bool _151 = false;
    if (_140)
    {
        _151 = (-_55) == (-5);
    }
    else
    {
        _151 = false;
    }
    ok = _151;
    bool _158 = false;
    if (_151)
    {
        int2 _154 = -_145;
        _158 = all(bool2(_154.x == int2(-5, 5).x, _154.y == int2(-5, 5).y));
    }
    else
    {
        _158 = false;
    }
    ok = _158;
    float4 _159 = 0.0f.xxxx;
    if (_158)
    {
        _159 = _10_colorGreen;
    }
    else
    {
        _159 = _10_colorRed;
    }
    return _159;
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
