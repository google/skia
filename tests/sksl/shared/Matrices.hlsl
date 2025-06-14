cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _13_colorGreen : packoffset(c0);
    float4 _13_colorRed : packoffset(c1);
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
    bool _51 = false;
    if (true)
    {
        _51 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _51 = false;
    }
    ok = _51;
    float2x2 m3 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _60 = false;
    if (_51)
    {
        _60 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _60 = false;
    }
    ok = _60;
    float2x2 m4 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
    bool _73 = false;
    if (_60)
    {
        _73 = all(bool2(float2(6.0f, 0.0f).x == float2(6.0f, 0.0f).x, float2(6.0f, 0.0f).y == float2(6.0f, 0.0f).y)) && all(bool2(float2(0.0f, 6.0f).x == float2(0.0f, 6.0f).x, float2(0.0f, 6.0f).y == float2(0.0f, 6.0f).y));
    }
    else
    {
        _73 = false;
    }
    ok = _73;
    float2x2 _74 = mul(float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f)), float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f)));
    m3 = _74;
    bool _90 = false;
    if (_73)
    {
        float2 _83 = _74[0];
        float2 _86 = _74[1];
        _90 = all(bool2(_83.x == float2(6.0f, 12.0f).x, _83.y == float2(6.0f, 12.0f).y)) && all(bool2(_86.x == float2(18.0f, 24.0f).x, _86.y == float2(18.0f, 24.0f).y));
    }
    else
    {
        _90 = false;
    }
    ok = _90;
    float2 _94 = m1[1];
    float2 _96 = float2(_94.y, 0.0f);
    float2 _97 = float2(0.0f, _94.y);
    float2x2 m5 = float2x2(_96, _97);
    bool _109 = false;
    if (_90)
    {
        _109 = all(bool2(_96.x == float2(4.0f, 0.0f).x, _96.y == float2(4.0f, 0.0f).y)) && all(bool2(_97.x == float2(0.0f, 4.0f).x, _97.y == float2(0.0f, 4.0f).y));
    }
    else
    {
        _109 = false;
    }
    ok = _109;
    float2 _110 = float2(1.0f, 2.0f) + _96;
    float2 _111 = float2(3.0f, 4.0f) + _97;
    m1 = float2x2(_110, _111);
    bool _125 = false;
    if (_109)
    {
        _125 = all(bool2(_110.x == float2(5.0f, 2.0f).x, _110.y == float2(5.0f, 2.0f).y)) && all(bool2(_111.x == float2(3.0f, 8.0f).x, _111.y == float2(3.0f, 8.0f).y));
    }
    else
    {
        _125 = false;
    }
    ok = _125;
    float2x2 m7 = float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f));
    bool _138 = false;
    if (_125)
    {
        _138 = all(bool2(float2(5.0f, 6.0f).x == float2(5.0f, 6.0f).x, float2(5.0f, 6.0f).y == float2(5.0f, 6.0f).y)) && all(bool2(float2(7.0f, 8.0f).x == float2(7.0f, 8.0f).x, float2(7.0f, 8.0f).y == float2(7.0f, 8.0f).y));
    }
    else
    {
        _138 = false;
    }
    ok = _138;
    float3x3 m9 = float3x3(float3(9.0f, 0.0f, 0.0f), float3(0.0f, 9.0f, 0.0f), float3(0.0f, 0.0f, 9.0f));
    bool _159 = false;
    if (_138)
    {
        _159 = (all(bool3(float3(9.0f, 0.0f, 0.0f).x == float3(9.0f, 0.0f, 0.0f).x, float3(9.0f, 0.0f, 0.0f).y == float3(9.0f, 0.0f, 0.0f).y, float3(9.0f, 0.0f, 0.0f).z == float3(9.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 9.0f, 0.0f).x == float3(0.0f, 9.0f, 0.0f).x, float3(0.0f, 9.0f, 0.0f).y == float3(0.0f, 9.0f, 0.0f).y, float3(0.0f, 9.0f, 0.0f).z == float3(0.0f, 9.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 9.0f).x == float3(0.0f, 0.0f, 9.0f).x, float3(0.0f, 0.0f, 9.0f).y == float3(0.0f, 0.0f, 9.0f).y, float3(0.0f, 0.0f, 9.0f).z == float3(0.0f, 0.0f, 9.0f).z));
    }
    else
    {
        _159 = false;
    }
    ok = _159;
    float4x4 m10 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
    bool _183 = false;
    if (_159)
    {
        _183 = ((all(bool4(float4(11.0f, 0.0f, 0.0f, 0.0f).x == float4(11.0f, 0.0f, 0.0f, 0.0f).x, float4(11.0f, 0.0f, 0.0f, 0.0f).y == float4(11.0f, 0.0f, 0.0f, 0.0f).y, float4(11.0f, 0.0f, 0.0f, 0.0f).z == float4(11.0f, 0.0f, 0.0f, 0.0f).z, float4(11.0f, 0.0f, 0.0f, 0.0f).w == float4(11.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 11.0f, 0.0f, 0.0f).x == float4(0.0f, 11.0f, 0.0f, 0.0f).x, float4(0.0f, 11.0f, 0.0f, 0.0f).y == float4(0.0f, 11.0f, 0.0f, 0.0f).y, float4(0.0f, 11.0f, 0.0f, 0.0f).z == float4(0.0f, 11.0f, 0.0f, 0.0f).z, float4(0.0f, 11.0f, 0.0f, 0.0f).w == float4(0.0f, 11.0f, 0.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 11.0f, 0.0f).x == float4(0.0f, 0.0f, 11.0f, 0.0f).x, float4(0.0f, 0.0f, 11.0f, 0.0f).y == float4(0.0f, 0.0f, 11.0f, 0.0f).y, float4(0.0f, 0.0f, 11.0f, 0.0f).z == float4(0.0f, 0.0f, 11.0f, 0.0f).z, float4(0.0f, 0.0f, 11.0f, 0.0f).w == float4(0.0f, 0.0f, 11.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 11.0f).x == float4(0.0f, 0.0f, 0.0f, 11.0f).x, float4(0.0f, 0.0f, 0.0f, 11.0f).y == float4(0.0f, 0.0f, 0.0f, 11.0f).y, float4(0.0f, 0.0f, 0.0f, 11.0f).z == float4(0.0f, 0.0f, 0.0f, 11.0f).z, float4(0.0f, 0.0f, 0.0f, 11.0f).w == float4(0.0f, 0.0f, 0.0f, 11.0f).w));
    }
    else
    {
        _183 = false;
    }
    ok = _183;
    float4x4 m11 = float4x4(20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx);
    float4 _188 = 20.0f.xxxx - float4(11.0f, 0.0f, 0.0f, 0.0f);
    float4 _189 = 20.0f.xxxx - float4(0.0f, 11.0f, 0.0f, 0.0f);
    float4 _190 = 20.0f.xxxx - float4(0.0f, 0.0f, 11.0f, 0.0f);
    float4 _191 = 20.0f.xxxx - float4(0.0f, 0.0f, 0.0f, 11.0f);
    m11 = float4x4(_188, _189, _190, _191);
    bool _211 = false;
    if (_183)
    {
        _211 = ((all(bool4(_188.x == float4(9.0f, 20.0f, 20.0f, 20.0f).x, _188.y == float4(9.0f, 20.0f, 20.0f, 20.0f).y, _188.z == float4(9.0f, 20.0f, 20.0f, 20.0f).z, _188.w == float4(9.0f, 20.0f, 20.0f, 20.0f).w)) && all(bool4(_189.x == float4(20.0f, 9.0f, 20.0f, 20.0f).x, _189.y == float4(20.0f, 9.0f, 20.0f, 20.0f).y, _189.z == float4(20.0f, 9.0f, 20.0f, 20.0f).z, _189.w == float4(20.0f, 9.0f, 20.0f, 20.0f).w))) && all(bool4(_190.x == float4(20.0f, 20.0f, 9.0f, 20.0f).x, _190.y == float4(20.0f, 20.0f, 9.0f, 20.0f).y, _190.z == float4(20.0f, 20.0f, 9.0f, 20.0f).z, _190.w == float4(20.0f, 20.0f, 9.0f, 20.0f).w))) && all(bool4(_191.x == float4(20.0f, 20.0f, 20.0f, 9.0f).x, _191.y == float4(20.0f, 20.0f, 20.0f, 9.0f).y, _191.z == float4(20.0f, 20.0f, 20.0f, 9.0f).z, _191.w == float4(20.0f, 20.0f, 20.0f, 9.0f).w));
    }
    else
    {
        _211 = false;
    }
    ok = _211;
    return _211;
}

bool test_comma_b()
{
    float2x2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    float2x2 y = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    return all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
}

float4 main(float2 _221)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_1_m1 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _232 = false;
    if (true)
    {
        _232 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _232 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _232;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m3 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    bool _241 = false;
    if (_232)
    {
        _241 = all(bool2(float2(1.0f, 2.0f).x == float2(1.0f, 2.0f).x, float2(1.0f, 2.0f).y == float2(1.0f, 2.0f).y)) && all(bool2(float2(3.0f, 4.0f).x == float2(3.0f, 4.0f).x, float2(3.0f, 4.0f).y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _241 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _241;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_3_m4 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
    float2x2 _243 = mul(float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f)), float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f)));
    _RESERVED_IDENTIFIER_FIXUP_2_m3 = _243;
    bool _253 = false;
    if (_241)
    {
        float2 _246 = _243[0];
        float2 _249 = _243[1];
        _253 = all(bool2(_246.x == float2(6.0f, 12.0f).x, _246.y == float2(6.0f, 12.0f).y)) && all(bool2(_249.x == float2(18.0f, 24.0f).x, _249.y == float2(18.0f, 24.0f).y));
    }
    else
    {
        _253 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _253;
    float2 _256 = _RESERVED_IDENTIFIER_FIXUP_1_m1[1];
    float2 _258 = float2(_256.y, 0.0f);
    float2 _259 = float2(0.0f, _256.y);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_4_m5 = float2x2(_258, _259);
    bool _268 = false;
    if (_253)
    {
        _268 = all(bool2(_258.x == float2(4.0f, 0.0f).x, _258.y == float2(4.0f, 0.0f).y)) && all(bool2(_259.x == float2(0.0f, 4.0f).x, _259.y == float2(0.0f, 4.0f).y));
    }
    else
    {
        _268 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _268;
    float2 _269 = float2(1.0f, 2.0f) + _258;
    float2 _270 = float2(3.0f, 4.0f) + _259;
    _RESERVED_IDENTIFIER_FIXUP_1_m1 = float2x2(_269, _270);
    bool _279 = false;
    if (_268)
    {
        _279 = all(bool2(_269.x == float2(5.0f, 2.0f).x, _269.y == float2(5.0f, 2.0f).y)) && all(bool2(_270.x == float2(3.0f, 8.0f).x, _270.y == float2(3.0f, 8.0f).y));
    }
    else
    {
        _279 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _279;
    float4x4 _RESERVED_IDENTIFIER_FIXUP_7_m10 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
    float4x4 _RESERVED_IDENTIFIER_FIXUP_8_m11 = float4x4(20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx, 20.0f.xxxx);
    float4 _282 = 20.0f.xxxx - float4(11.0f, 0.0f, 0.0f, 0.0f);
    float4 _283 = 20.0f.xxxx - float4(0.0f, 11.0f, 0.0f, 0.0f);
    float4 _284 = 20.0f.xxxx - float4(0.0f, 0.0f, 11.0f, 0.0f);
    float4 _285 = 20.0f.xxxx - float4(0.0f, 0.0f, 0.0f, 11.0f);
    _RESERVED_IDENTIFIER_FIXUP_8_m11 = float4x4(_282, _283, _284, _285);
    bool _300 = false;
    if (_279)
    {
        _300 = ((all(bool4(_282.x == float4(9.0f, 20.0f, 20.0f, 20.0f).x, _282.y == float4(9.0f, 20.0f, 20.0f, 20.0f).y, _282.z == float4(9.0f, 20.0f, 20.0f, 20.0f).z, _282.w == float4(9.0f, 20.0f, 20.0f, 20.0f).w)) && all(bool4(_283.x == float4(20.0f, 9.0f, 20.0f, 20.0f).x, _283.y == float4(20.0f, 9.0f, 20.0f, 20.0f).y, _283.z == float4(20.0f, 9.0f, 20.0f, 20.0f).z, _283.w == float4(20.0f, 9.0f, 20.0f, 20.0f).w))) && all(bool4(_284.x == float4(20.0f, 20.0f, 9.0f, 20.0f).x, _284.y == float4(20.0f, 20.0f, 9.0f, 20.0f).y, _284.z == float4(20.0f, 20.0f, 9.0f, 20.0f).z, _284.w == float4(20.0f, 20.0f, 9.0f, 20.0f).w))) && all(bool4(_285.x == float4(20.0f, 20.0f, 20.0f, 9.0f).x, _285.y == float4(20.0f, 20.0f, 20.0f, 9.0f).y, _285.z == float4(20.0f, 20.0f, 20.0f, 9.0f).z, _285.w == float4(20.0f, 20.0f, 20.0f, 9.0f).w));
    }
    else
    {
        _300 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _300;
    bool _304 = false;
    if (_300)
    {
        _304 = test_half_b();
    }
    else
    {
        _304 = false;
    }
    bool _308 = false;
    if (_304)
    {
        _308 = test_comma_b();
    }
    else
    {
        _308 = false;
    }
    float4 _309 = 0.0f.xxxx;
    if (_308)
    {
        _309 = _13_colorGreen;
    }
    else
    {
        _309 = _13_colorRed;
    }
    return _309;
}

void frag_main()
{
    float2 _23 = 0.0f.xx;
    sk_FragColor = main(_23);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
