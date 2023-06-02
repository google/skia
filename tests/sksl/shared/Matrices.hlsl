cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
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
    bool _49 = false;
    if (true)
    {
        _49 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _49 = false;
    }
    ok = _49;
    float2x2 m3 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _58 = false;
    if (_49)
    {
        _58 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _58 = false;
    }
    ok = _58;
    float2x2 m4 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
    bool _71 = false;
    if (_58)
    {
        _71 = all(bool2(float2(6.0f, 0.0f).x == float2(6.0f, 0.0f).x, float2(6.0f, 0.0f).y == float2(6.0f, 0.0f).y)) && all(bool2(float2(0.0f, 6.0f).x == float2(0.0f, 6.0f).x, float2(0.0f, 6.0f).y == float2(0.0f, 6.0f).y));
    }
    else
    {
        _71 = false;
    }
    ok = _71;
    float2x2 _72 = mul(float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f)), float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f)));
    m3 = _72;
    bool _88 = false;
    if (_71)
    {
        float2 _81 = _72[0];
        float2 _84 = _72[1];
        _88 = all(bool2(_81.x == float2(6.0f, 12.0f).x, _81.y == float2(6.0f, 12.0f).y)) && all(bool2(_84.x == float2(18.0f, 24.0f).x, _84.y == float2(18.0f, 24.0f).y));
    }
    else
    {
        _88 = false;
    }
    ok = _88;
    float2 _93 = m1[1];
    float2 _95 = float2(_93.y, 0.0f);
    float2 _96 = float2(0.0f, _93.y);
    float2x2 m5 = float2x2(_95, _96);
    bool _108 = false;
    if (_88)
    {
        _108 = all(bool2(_95.x == float2(4.0f, 0.0f).x, _95.y == float2(4.0f, 0.0f).y)) && all(bool2(_96.x == float2(0.0f, 4.0f).x, _96.y == float2(0.0f, 4.0f).y));
    }
    else
    {
        _108 = false;
    }
    ok = _108;
    float2 _109 = float2(1.0f, 2.0f) + _95;
    float2 _110 = float2(3.0f, 4.0f) + _96;
    m1 = float2x2(_109, _110);
    bool _124 = false;
    if (_108)
    {
        _124 = all(bool2(_109.x == float2(5.0f, 2.0f).x, _109.y == float2(5.0f, 2.0f).y)) && all(bool2(_110.x == float2(3.0f, 8.0f).x, _110.y == float2(3.0f, 8.0f).y));
    }
    else
    {
        _124 = false;
    }
    ok = _124;
    float2x2 m7 = float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f));
    bool _137 = false;
    if (_124)
    {
        _137 = all(bool2(float2(5.0f, 6.0f).x == float2(5.0f, 6.0f).x, float2(5.0f, 6.0f).y == float2(5.0f, 6.0f).y)) && all(bool2(float2(7.0f, 8.0f).x == float2(7.0f, 8.0f).x, float2(7.0f, 8.0f).y == float2(7.0f, 8.0f).y));
    }
    else
    {
        _137 = false;
    }
    ok = _137;
    float3x3 m9 = float3x3(float3(9.0f, 0.0f, 0.0f), float3(0.0f, 9.0f, 0.0f), float3(0.0f, 0.0f, 9.0f));
    bool _158 = false;
    if (_137)
    {
        _158 = (all(bool3(float3(9.0f, 0.0f, 0.0f).x == float3(9.0f, 0.0f, 0.0f).x, float3(9.0f, 0.0f, 0.0f).y == float3(9.0f, 0.0f, 0.0f).y, float3(9.0f, 0.0f, 0.0f).z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 9.0f, 0.0f).x == float3(0.0f, 9.0f, 0.0f).x, float3(0.0f, 9.0f, 0.0f).y == float3(0.0f, 9.0f, 0.0f).y, float3(0.0f, 9.0f, 0.0f).z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 9.0f).x == float3(0.0f, 0.0f, 9.0f).x, float3(0.0f, 0.0f, 9.0f).y == float3(0.0f, 0.0f, 9.0f).y, float3(0.0f, 0.0f, 9.0f).z == float3(0.0f, 0.0f, 9.0f).z));
    }
    else
    {
        _158 = false;
    }
    ok = _158;
    float4x4 m10 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
    bool _182 = false;
    if (_158)
    {
        _182 = ((all(bool4(float4(11.0f, 0.0f, 0.0f, 0.0f).x == float4(11.0f, 0.0f, 0.0f, 0.0f).x, float4(11.0f, 0.0f, 0.0f, 0.0f).y == float4(11.0f, 0.0f, 0.0f, 0.0f).y, float4(11.0f, 0.0f, 0.0f, 0.0f).z == float4(11.0f, 0.0f, 0.0f, 0.0f).z, float4(11.0f, 0.0f, 0.0f, 0.0f).w == float4(11.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 11.0f, 0.0f, 0.0f).x == float4(0.0f, 11.0f, 0.0f, 0.0f).x, float4(0.0f, 11.0f, 0.0f, 0.0f).y == float4(0.0f, 11.0f, 0.0f, 0.0f).y, float4(0.0f, 11.0f, 0.0f, 0.0f).z == float4(0.0f, 11.0f, 0.0f, 0.0f).z, float4(0.0f, 11.0f, 0.0f, 0.0f).w == float4(0.0f, 11.0f, 0.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 11.0f, 0.0f).x == float4(0.0f, 0.0f, 11.0f, 0.0f).x, float4(0.0f, 0.0f, 11.0f, 0.0f).y == float4(0.0f, 0.0f, 11.0f, 0.0f).y, float4(0.0f, 0.0f, 11.0f, 0.0f).z == float4(0.0f, 0.0f, 11.0f, 0.0f).z, float4(0.0f, 0.0f, 11.0f, 0.0f).w == float4(0.0f, 0.0f, 11.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 11.0f).x == float4(0.0f, 0.0f, 0.0f, 11.0f).x, float4(0.0f, 0.0f, 0.0f, 11.0f).y == float4(0.0f, 0.0f, 0.0f, 11.0f).y, float4(0.0f, 0.0f, 0.0f, 11.0f).z == float4(0.0f, 0.0f, 0.0f, 11.0f).z, float4(0.0f, 0.0f, 0.0f, 11.0f).w == float4(0.0f, 0.0f, 0.0f, 11.0f).w));
    }
    else
    {
        _182 = false;
    }
    ok = _182;
    float4x4 m11 = float4x4(20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx);
    float4 _187 = 20.0f.xxxx - float4(11.0f, 0.0f, 0.0f, 0.0f);
    float4 _188 = 20.0f.xxxx - float4(0.0f, 11.0f, 0.0f, 0.0f);
    float4 _189 = 20.0f.xxxx - float4(0.0f, 0.0f, 11.0f, 0.0f);
    float4 _190 = 20.0f.xxxx - float4(0.0f, 0.0f, 0.0f, 11.0f);
    m11 = float4x4(_187, _188, _189, _190);
    bool _210 = false;
    if (_182)
    {
        _210 = ((all(bool4(_187.x == float4(9.0f, 20.0f, 20.0f, 20.0f).x, _187.y == float4(9.0f, 20.0f, 20.0f, 20.0f).y, _187.z == float4(9.0f, 20.0f, 20.0f, 20.0f).z, _187.w == float4(9.0f, 20.0f, 20.0f, 20.0f).w)) && all(bool4(_188.x == float4(20.0f, 9.0f, 20.0f, 20.0f).x, _188.y == float4(20.0f, 9.0f, 20.0f, 20.0f).y, _188.z == float4(20.0f, 9.0f, 20.0f, 20.0f).z, _188.w == float4(20.0f, 9.0f, 20.0f, 20.0f).w))) && all(bool4(_189.x == float4(20.0f, 20.0f, 9.0f, 20.0f).x, _189.y == float4(20.0f, 20.0f, 9.0f, 20.0f).y, _189.z == float4(20.0f, 20.0f, 9.0f, 20.0f).z, _189.w == float4(20.0f, 20.0f, 9.0f, 20.0f).w))) && all(bool4(_190.x == float4(20.0f, 20.0f, 20.0f, 9.0f).x, _190.y == float4(20.0f, 20.0f, 20.0f, 9.0f).y, _190.z == float4(20.0f, 20.0f, 20.0f, 9.0f).z, _190.w == float4(20.0f, 20.0f, 20.0f, 9.0f).w));
    }
    else
    {
        _210 = false;
    }
    ok = _210;
    return _210;
}

bool test_comma_b()
{
    float2x2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    float2x2 y = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    return all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
}

float4 main(float2 _220)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_1_m1 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _231 = false;
    if (true)
    {
        _231 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _231 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _231;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m3 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _240 = false;
    if (_231)
    {
        _240 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _240 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _240;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_3_m4 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
    float2x2 _242 = mul(float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f)), float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f)));
    _RESERVED_IDENTIFIER_FIXUP_2_m3 = _242;
    bool _252 = false;
    if (_240)
    {
        float2 _245 = _242[0];
        float2 _248 = _242[1];
        _252 = all(bool2(_245.x == float2(6.0f, 12.0f).x, _245.y == float2(6.0f, 12.0f).y)) && all(bool2(_248.x == float2(18.0f, 24.0f).x, _248.y == float2(18.0f, 24.0f).y));
    }
    else
    {
        _252 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _252;
    float2 _255 = _RESERVED_IDENTIFIER_FIXUP_1_m1[1];
    float2 _257 = float2(_255.y, 0.0f);
    float2 _258 = float2(0.0f, _255.y);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_4_m5 = float2x2(_257, _258);
    bool _267 = false;
    if (_252)
    {
        _267 = all(bool2(_257.x == float2(4.0f, 0.0f).x, _257.y == float2(4.0f, 0.0f).y)) && all(bool2(_258.x == float2(0.0f, 4.0f).x, _258.y == float2(0.0f, 4.0f).y));
    }
    else
    {
        _267 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _267;
    float2 _268 = float2(1.0f, 2.0f) + _257;
    float2 _269 = float2(3.0f, 4.0f) + _258;
    _RESERVED_IDENTIFIER_FIXUP_1_m1 = float2x2(_268, _269);
    bool _278 = false;
    if (_267)
    {
        _278 = all(bool2(_268.x == float2(5.0f, 2.0f).x, _268.y == float2(5.0f, 2.0f).y)) && all(bool2(_269.x == float2(3.0f, 8.0f).x, _269.y == float2(3.0f, 8.0f).y));
    }
    else
    {
        _278 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _278;
    float4x4 _RESERVED_IDENTIFIER_FIXUP_7_m10 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
    float4x4 _RESERVED_IDENTIFIER_FIXUP_8_m11 = float4x4(20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx);
    float4 _281 = 20.0f.xxxx - float4(11.0f, 0.0f, 0.0f, 0.0f);
    float4 _282 = 20.0f.xxxx - float4(0.0f, 11.0f, 0.0f, 0.0f);
    float4 _283 = 20.0f.xxxx - float4(0.0f, 0.0f, 11.0f, 0.0f);
    float4 _284 = 20.0f.xxxx - float4(0.0f, 0.0f, 0.0f, 11.0f);
    _RESERVED_IDENTIFIER_FIXUP_8_m11 = float4x4(_281, _282, _283, _284);
    bool _299 = false;
    if (_278)
    {
        _299 = ((all(bool4(_281.x == float4(9.0f, 20.0f, 20.0f, 20.0f).x, _281.y == float4(9.0f, 20.0f, 20.0f, 20.0f).y, _281.z == float4(9.0f, 20.0f, 20.0f, 20.0f).z, _281.w == float4(9.0f, 20.0f, 20.0f, 20.0f).w)) && all(bool4(_282.x == float4(20.0f, 9.0f, 20.0f, 20.0f).x, _282.y == float4(20.0f, 9.0f, 20.0f, 20.0f).y, _282.z == float4(20.0f, 9.0f, 20.0f, 20.0f).z, _282.w == float4(20.0f, 9.0f, 20.0f, 20.0f).w))) && all(bool4(_283.x == float4(20.0f, 20.0f, 9.0f, 20.0f).x, _283.y == float4(20.0f, 20.0f, 9.0f, 20.0f).y, _283.z == float4(20.0f, 20.0f, 9.0f, 20.0f).z, _283.w == float4(20.0f, 20.0f, 9.0f, 20.0f).w))) && all(bool4(_284.x == float4(20.0f, 20.0f, 20.0f, 9.0f).x, _284.y == float4(20.0f, 20.0f, 20.0f, 9.0f).y, _284.z == float4(20.0f, 20.0f, 20.0f, 9.0f).z, _284.w == float4(20.0f, 20.0f, 20.0f, 9.0f).w));
    }
    else
    {
        _299 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _299;
    bool _303 = false;
    if (_299)
    {
        _303 = test_half_b();
    }
    else
    {
        _303 = false;
    }
    bool _307 = false;
    if (_303)
    {
        _307 = test_comma_b();
    }
    else
    {
        _307 = false;
    }
    float4 _308 = 0.0f.xxxx;
    if (_307)
    {
        _308 = _12_colorGreen;
    }
    else
    {
        _308 = _12_colorRed;
    }
    return _308;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
