cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    bool ok = true;
    int i = 5;
    int _32 = 5 + 1;
    i = _32;
    bool _39 = false;
    if (true)
    {
        i = _32 + 1;
        _39 = _32 == 6;
    }
    else
    {
        _39 = false;
    }
    ok = _39;
    bool _45 = false;
    if (_39)
    {
        _45 = i == 7;
    }
    else
    {
        _45 = false;
    }
    ok = _45;
    bool _51 = false;
    if (_45)
    {
        int _48 = i;
        i = _48 - 1;
        _51 = _48 == 7;
    }
    else
    {
        _51 = false;
    }
    ok = _51;
    bool _56 = false;
    if (_51)
    {
        _56 = i == 6;
    }
    else
    {
        _56 = false;
    }
    ok = _56;
    int _57 = i;
    int _58 = _57 - 1;
    i = _58;
    bool _62 = false;
    if (_56)
    {
        _62 = _58 == 5;
    }
    else
    {
        _62 = false;
    }
    ok = _62;
    float f = 0.5f;
    float _67 = 0.5f + 1.0f;
    f = _67;
    bool _73 = false;
    if (_62)
    {
        f = _67 + 1.0f;
        _73 = _67 == 1.5f;
    }
    else
    {
        _73 = false;
    }
    ok = _73;
    bool _79 = false;
    if (_73)
    {
        _79 = f == 2.5f;
    }
    else
    {
        _79 = false;
    }
    ok = _79;
    bool _85 = false;
    if (_79)
    {
        float _82 = f;
        f = _82 - 1.0f;
        _85 = _82 == 2.5f;
    }
    else
    {
        _85 = false;
    }
    ok = _85;
    bool _90 = false;
    if (_85)
    {
        _90 = f == 1.5f;
    }
    else
    {
        _90 = false;
    }
    ok = _90;
    float _91 = f;
    float _92 = _91 - 1.0f;
    f = _92;
    bool _96 = false;
    if (_90)
    {
        _96 = _92 == 0.5f;
    }
    else
    {
        _96 = false;
    }
    ok = _96;
    float2 f2 = 0.5f.xx;
    f2.x += 1.0f;
    bool _110 = false;
    if (ok)
    {
        float _107 = f2.x;
        f2.x = _107 + 1.0f;
        _110 = _107 == 1.5f;
    }
    else
    {
        _110 = false;
    }
    ok = _110;
    bool _116 = false;
    if (_110)
    {
        _116 = f2.x == 2.5f;
    }
    else
    {
        _116 = false;
    }
    ok = _116;
    bool _123 = false;
    if (_116)
    {
        float _120 = f2.x;
        f2.x = _120 - 1.0f;
        _123 = _120 == 2.5f;
    }
    else
    {
        _123 = false;
    }
    ok = _123;
    bool _129 = false;
    if (_123)
    {
        _129 = f2.x == 1.5f;
    }
    else
    {
        _129 = false;
    }
    ok = _129;
    f2.x -= 1.0f;
    bool _139 = false;
    if (ok)
    {
        _139 = f2.x == 0.5f;
    }
    else
    {
        _139 = false;
    }
    ok = _139;
    float4 _140 = 0.0f.xxxx;
    if (_139)
    {
        _140 = _7_colorGreen;
    }
    else
    {
        _140 = _7_colorRed;
    }
    return _140;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
