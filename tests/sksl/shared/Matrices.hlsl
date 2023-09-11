cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _9_colorGreen : packoffset(c0);
    float4 _9_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_half_b()
{
    bool ok = true;
    float2x2 m1 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _47 = false;
    if (true)
    {
        _47 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _47 = false;
    }
    ok = _47;
    float2x2 m3 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _56 = false;
    if (_47)
    {
        _56 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _56 = false;
    }
    ok = _56;
    float2x2 m4 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
    bool _69 = false;
    if (_56)
    {
        _69 = all(bool2(float2(6.0f, 0.0f).x == float2(6.0f, 0.0f).x, float2(6.0f, 0.0f).y == float2(6.0f, 0.0f).y)) && all(bool2(float2(0.0f, 6.0f).x == float2(0.0f, 6.0f).x, float2(0.0f, 6.0f).y == float2(0.0f, 6.0f).y));
    }
    else
    {
        _69 = false;
    }
    ok = _69;
    float2x2 _70 = mul(float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f)), float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f)));
    m3 = _70;
    bool _86 = false;
    if (_69)
    {
        float2 _79 = _70[0];
        float2 _82 = _70[1];
        _86 = all(bool2(_79.x == float2(6.0f, 12.0f).x, _79.y == float2(6.0f, 12.0f).y)) && all(bool2(_82.x == float2(18.0f, 24.0f).x, _82.y == float2(18.0f, 24.0f).y));
    }
    else
    {
        _86 = false;
    }
    ok = _86;
    float2 _91 = m1[1];
    float2 _93 = float2(_91.y, 0.0f);
    float2 _94 = float2(0.0f, _91.y);
    float2x2 m5 = float2x2(_93, _94);
    bool _106 = false;
    if (_86)
    {
        _106 = all(bool2(_93.x == float2(4.0f, 0.0f).x, _93.y == float2(4.0f, 0.0f).y)) && all(bool2(_94.x == float2(0.0f, 4.0f).x, _94.y == float2(0.0f, 4.0f).y));
    }
    else
    {
        _106 = false;
    }
    ok = _106;
    float2 _107 = float2(1.0f, 2.0f) + _93;
    float2 _108 = float2(3.0f, 4.0f) + _94;
    m1 = float2x2(_107, _108);
    bool _122 = false;
    if (_106)
    {
        _122 = all(bool2(_107.x == float2(5.0f, 2.0f).x, _107.y == float2(5.0f, 2.0f).y)) && all(bool2(_108.x == float2(3.0f, 8.0f).x, _108.y == float2(3.0f, 8.0f).y));
    }
    else
    {
        _122 = false;
    }
    ok = _122;
    float2x2 m7 = float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f));
    bool _135 = false;
    if (_122)
    {
        _135 = all(bool2(float2(5.0f, 6.0f).x == float2(5.0f, 6.0f).x, float2(5.0f, 6.0f).y == float2(5.0f, 6.0f).y)) && all(bool2(float2(7.0f, 8.0f).x == float2(7.0f, 8.0f).x, float2(7.0f, 8.0f).y == float2(7.0f, 8.0f).y));
    }
    else
    {
        _135 = false;
    }
    ok = _135;
    float3x3 m9 = float3x3(float3(9.0f, 0.0f, 0.0f), float3(0.0f, 9.0f, 0.0f), float3(0.0f, 0.0f, 9.0f));
    bool _156 = false;
    if (_135)
    {
        _156 = (all(bool3(float3(9.0f, 0.0f, 0.0f).x == float3(9.0f, 0.0f, 0.0f).x, float3(9.0f, 0.0f, 0.0f).y == float3(9.0f, 0.0f, 0.0f).y, float3(9.0f, 0.0f, 0.0f).z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 9.0f, 0.0f).x == float3(0.0f, 9.0f, 0.0f).x, float3(0.0f, 9.0f, 0.0f).y == float3(0.0f, 9.0f, 0.0f).y, float3(0.0f, 9.0f, 0.0f).z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 9.0f).x == float3(0.0f, 0.0f, 9.0f).x, float3(0.0f, 0.0f, 9.0f).y == float3(0.0f, 0.0f, 9.0f).y, float3(0.0f, 0.0f, 9.0f).z == float3(0.0f, 0.0f, 9.0f).z));
    }
    else
    {
        _156 = false;
    }
    ok = _156;
    float4x4 m10 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
    bool _180 = false;
    if (_156)
    {
        _180 = ((all(bool4(float4(11.0f, 0.0f, 0.0f, 0.0f).x == float4(11.0f, 0.0f, 0.0f, 0.0f).x, float4(11.0f, 0.0f, 0.0f, 0.0f).y == float4(11.0f, 0.0f, 0.0f, 0.0f).y, float4(11.0f, 0.0f, 0.0f, 0.0f).z == float4(11.0f, 0.0f, 0.0f, 0.0f).z, float4(11.0f, 0.0f, 0.0f, 0.0f).w == float4(11.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 11.0f, 0.0f, 0.0f).x == float4(0.0f, 11.0f, 0.0f, 0.0f).x, float4(0.0f, 11.0f, 0.0f, 0.0f).y == float4(0.0f, 11.0f, 0.0f, 0.0f).y, float4(0.0f, 11.0f, 0.0f, 0.0f).z == float4(0.0f, 11.0f, 0.0f, 0.0f).z, float4(0.0f, 11.0f, 0.0f, 0.0f).w == float4(0.0f, 11.0f, 0.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 11.0f, 0.0f).x == float4(0.0f, 0.0f, 11.0f, 0.0f).x, float4(0.0f, 0.0f, 11.0f, 0.0f).y == float4(0.0f, 0.0f, 11.0f, 0.0f).y, float4(0.0f, 0.0f, 11.0f, 0.0f).z == float4(0.0f, 0.0f, 11.0f, 0.0f).z, float4(0.0f, 0.0f, 11.0f, 0.0f).w == float4(0.0f, 0.0f, 11.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 11.0f).x == float4(0.0f, 0.0f, 0.0f, 11.0f).x, float4(0.0f, 0.0f, 0.0f, 11.0f).y == float4(0.0f, 0.0f, 0.0f, 11.0f).y, float4(0.0f, 0.0f, 0.0f, 11.0f).z == float4(0.0f, 0.0f, 0.0f, 11.0f).z, float4(0.0f, 0.0f, 0.0f, 11.0f).w == float4(0.0f, 0.0f, 0.0f, 11.0f).w));
    }
    else
    {
        _180 = false;
    }
    ok = _180;
    float4x4 m11 = float4x4(20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx);
    float4 _185 = 20.0f.xxxx - float4(11.0f, 0.0f, 0.0f, 0.0f);
    float4 _186 = 20.0f.xxxx - float4(0.0f, 11.0f, 0.0f, 0.0f);
    float4 _187 = 20.0f.xxxx - float4(0.0f, 0.0f, 11.0f, 0.0f);
    float4 _188 = 20.0f.xxxx - float4(0.0f, 0.0f, 0.0f, 11.0f);
    m11 = float4x4(_185, _186, _187, _188);
    bool _208 = false;
    if (_180)
    {
        _208 = ((all(bool4(_185.x == float4(9.0f, 20.0f, 20.0f, 20.0f).x, _185.y == float4(9.0f, 20.0f, 20.0f, 20.0f).y, _185.z == float4(9.0f, 20.0f, 20.0f, 20.0f).z, _185.w == float4(9.0f, 20.0f, 20.0f, 20.0f).w)) && all(bool4(_186.x == float4(20.0f, 9.0f, 20.0f, 20.0f).x, _186.y == float4(20.0f, 9.0f, 20.0f, 20.0f).y, _186.z == float4(20.0f, 9.0f, 20.0f, 20.0f).z, _186.w == float4(20.0f, 9.0f, 20.0f, 20.0f).w))) && all(bool4(_187.x == float4(20.0f, 20.0f, 9.0f, 20.0f).x, _187.y == float4(20.0f, 20.0f, 9.0f, 20.0f).y, _187.z == float4(20.0f, 20.0f, 9.0f, 20.0f).z, _187.w == float4(20.0f, 20.0f, 9.0f, 20.0f).w))) && all(bool4(_188.x == float4(20.0f, 20.0f, 20.0f, 9.0f).x, _188.y == float4(20.0f, 20.0f, 20.0f, 9.0f).y, _188.z == float4(20.0f, 20.0f, 20.0f, 9.0f).z, _188.w == float4(20.0f, 20.0f, 20.0f, 9.0f).w));
    }
    else
    {
        _208 = false;
    }
    ok = _208;
    return _208;
}

bool test_comma_b()
{
    float2x2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    float2x2 y = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    return all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
}

float4 main(float2 _218)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_1_m1 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _229 = false;
    if (true)
    {
        _229 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _229 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _229;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m3 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _238 = false;
    if (_229)
    {
        _238 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _238 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _238;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_3_m4 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
    float2x2 _240 = mul(float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f)), float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f)));
    _RESERVED_IDENTIFIER_FIXUP_2_m3 = _240;
    bool _250 = false;
    if (_238)
    {
        float2 _243 = _240[0];
        float2 _246 = _240[1];
        _250 = all(bool2(_243.x == float2(6.0f, 12.0f).x, _243.y == float2(6.0f, 12.0f).y)) && all(bool2(_246.x == float2(18.0f, 24.0f).x, _246.y == float2(18.0f, 24.0f).y));
    }
    else
    {
        _250 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _250;
    float2 _253 = _RESERVED_IDENTIFIER_FIXUP_1_m1[1];
    float2 _255 = float2(_253.y, 0.0f);
    float2 _256 = float2(0.0f, _253.y);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_4_m5 = float2x2(_255, _256);
    bool _265 = false;
    if (_250)
    {
        _265 = all(bool2(_255.x == float2(4.0f, 0.0f).x, _255.y == float2(4.0f, 0.0f).y)) && all(bool2(_256.x == float2(0.0f, 4.0f).x, _256.y == float2(0.0f, 4.0f).y));
    }
    else
    {
        _265 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _265;
    float2 _266 = float2(1.0f, 2.0f) + _255;
    float2 _267 = float2(3.0f, 4.0f) + _256;
    _RESERVED_IDENTIFIER_FIXUP_1_m1 = float2x2(_266, _267);
    bool _276 = false;
    if (_265)
    {
        _276 = all(bool2(_266.x == float2(5.0f, 2.0f).x, _266.y == float2(5.0f, 2.0f).y)) && all(bool2(_267.x == float2(3.0f, 8.0f).x, _267.y == float2(3.0f, 8.0f).y));
    }
    else
    {
        _276 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _276;
    float4x4 _RESERVED_IDENTIFIER_FIXUP_7_m10 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
    float4x4 _RESERVED_IDENTIFIER_FIXUP_8_m11 = float4x4(20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx);
    float4 _279 = 20.0f.xxxx - float4(11.0f, 0.0f, 0.0f, 0.0f);
    float4 _280 = 20.0f.xxxx - float4(0.0f, 11.0f, 0.0f, 0.0f);
    float4 _281 = 20.0f.xxxx - float4(0.0f, 0.0f, 11.0f, 0.0f);
    float4 _282 = 20.0f.xxxx - float4(0.0f, 0.0f, 0.0f, 11.0f);
    _RESERVED_IDENTIFIER_FIXUP_8_m11 = float4x4(_279, _280, _281, _282);
    bool _297 = false;
    if (_276)
    {
        _297 = ((all(bool4(_279.x == float4(9.0f, 20.0f, 20.0f, 20.0f).x, _279.y == float4(9.0f, 20.0f, 20.0f, 20.0f).y, _279.z == float4(9.0f, 20.0f, 20.0f, 20.0f).z, _279.w == float4(9.0f, 20.0f, 20.0f, 20.0f).w)) && all(bool4(_280.x == float4(20.0f, 9.0f, 20.0f, 20.0f).x, _280.y == float4(20.0f, 9.0f, 20.0f, 20.0f).y, _280.z == float4(20.0f, 9.0f, 20.0f, 20.0f).z, _280.w == float4(20.0f, 9.0f, 20.0f, 20.0f).w))) && all(bool4(_281.x == float4(20.0f, 20.0f, 9.0f, 20.0f).x, _281.y == float4(20.0f, 20.0f, 9.0f, 20.0f).y, _281.z == float4(20.0f, 20.0f, 9.0f, 20.0f).z, _281.w == float4(20.0f, 20.0f, 9.0f, 20.0f).w))) && all(bool4(_282.x == float4(20.0f, 20.0f, 20.0f, 9.0f).x, _282.y == float4(20.0f, 20.0f, 20.0f, 9.0f).y, _282.z == float4(20.0f, 20.0f, 20.0f, 9.0f).z, _282.w == float4(20.0f, 20.0f, 20.0f, 9.0f).w));
    }
    else
    {
        _297 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _297;
    bool _301 = false;
    if (_297)
    {
        _301 = test_half_b();
    }
    else
    {
        _301 = false;
    }
    bool _305 = false;
    if (_301)
    {
        _305 = test_comma_b();
    }
    else
    {
        _305 = false;
    }
    float4 _306 = 0.0f.xxxx;
    if (_305)
    {
        _306 = _9_colorGreen;
    }
    else
    {
        _306 = _9_colorRed;
    }
    return _306;
}

void frag_main()
{
    float2 _19 = 0.0f.xx;
    sk_FragColor = main(_19);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
