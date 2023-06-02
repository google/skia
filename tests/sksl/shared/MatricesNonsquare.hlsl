cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_half_b()
{
    bool ok = true;
    float2x3 m23 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
    bool _46 = false;
    if (true)
    {
        _46 = all(bool3(float3(2.0f, 0.0f, 0.0f).x == float3(2.0f, 0.0f, 0.0f).x, float3(2.0f, 0.0f, 0.0f).y == float3(2.0f, 0.0f, 0.0f).y, float3(2.0f, 0.0f, 0.0f).z == float3(2.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 2.0f, 0.0f).x == float3(0.0f, 2.0f, 0.0f).x, float3(0.0f, 2.0f, 0.0f).y == float3(0.0f, 2.0f, 0.0f).y, float3(0.0f, 2.0f, 0.0f).z == float3(0.0f, 2.0f, 0.0f).z));
    }
    else
    {
        _46 = false;
    }
    ok = _46;
    float2x4 m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
    bool _62 = false;
    if (_46)
    {
        _62 = all(bool4(float4(3.0f, 0.0f, 0.0f, 0.0f).x == float4(3.0f, 0.0f, 0.0f, 0.0f).x, float4(3.0f, 0.0f, 0.0f, 0.0f).y == float4(3.0f, 0.0f, 0.0f, 0.0f).y, float4(3.0f, 0.0f, 0.0f, 0.0f).z == float4(3.0f, 0.0f, 0.0f, 0.0f).z, float4(3.0f, 0.0f, 0.0f, 0.0f).w == float4(3.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 3.0f, 0.0f, 0.0f).x == float4(0.0f, 3.0f, 0.0f, 0.0f).x, float4(0.0f, 3.0f, 0.0f, 0.0f).y == float4(0.0f, 3.0f, 0.0f, 0.0f).y, float4(0.0f, 3.0f, 0.0f, 0.0f).z == float4(0.0f, 3.0f, 0.0f, 0.0f).z, float4(0.0f, 3.0f, 0.0f, 0.0f).w == float4(0.0f, 3.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _62 = false;
    }
    ok = _62;
    float3x2 m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
    bool _81 = false;
    if (_62)
    {
        _81 = (all(bool2(float2(4.0f, 0.0f).x == float2(4.0f, 0.0f).x, float2(4.0f, 0.0f).y == float2(4.0f, 0.0f).y)) && all(bool2(float2(0.0f, 4.0f).x == float2(0.0f, 4.0f).x, float2(0.0f, 4.0f).y == float2(0.0f, 4.0f).y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y));
    }
    else
    {
        _81 = false;
    }
    ok = _81;
    float3x4 m34 = float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f));
    bool _100 = false;
    if (_81)
    {
        _100 = (all(bool4(float4(5.0f, 0.0f, 0.0f, 0.0f).x == float4(5.0f, 0.0f, 0.0f, 0.0f).x, float4(5.0f, 0.0f, 0.0f, 0.0f).y == float4(5.0f, 0.0f, 0.0f, 0.0f).y, float4(5.0f, 0.0f, 0.0f, 0.0f).z == float4(5.0f, 0.0f, 0.0f, 0.0f).z, float4(5.0f, 0.0f, 0.0f, 0.0f).w == float4(5.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 5.0f, 0.0f, 0.0f).x == float4(0.0f, 5.0f, 0.0f, 0.0f).x, float4(0.0f, 5.0f, 0.0f, 0.0f).y == float4(0.0f, 5.0f, 0.0f, 0.0f).y, float4(0.0f, 5.0f, 0.0f, 0.0f).z == float4(0.0f, 5.0f, 0.0f, 0.0f).z, float4(0.0f, 5.0f, 0.0f, 0.0f).w == float4(0.0f, 5.0f, 0.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 5.0f, 0.0f).x == float4(0.0f, 0.0f, 5.0f, 0.0f).x, float4(0.0f, 0.0f, 5.0f, 0.0f).y == float4(0.0f, 0.0f, 5.0f, 0.0f).y, float4(0.0f, 0.0f, 5.0f, 0.0f).z == float4(0.0f, 0.0f, 5.0f, 0.0f).z, float4(0.0f, 0.0f, 5.0f, 0.0f).w == float4(0.0f, 0.0f, 5.0f, 0.0f).w));
    }
    else
    {
        _100 = false;
    }
    ok = _100;
    float4x2 m42 = float4x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f), 0.0f.xx, 0.0f.xx);
    bool _121 = false;
    if (_100)
    {
        _121 = ((all(bool2(float2(6.0f, 0.0f).x == float2(6.0f, 0.0f).x, float2(6.0f, 0.0f).y == float2(6.0f, 0.0f).y)) && all(bool2(float2(0.0f, 6.0f).x == float2(0.0f, 6.0f).x, float2(0.0f, 6.0f).y == float2(0.0f, 6.0f).y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y));
    }
    else
    {
        _121 = false;
    }
    ok = _121;
    float4x3 m43 = float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), 0.0f.xxx);
    bool _144 = false;
    if (_121)
    {
        _144 = ((all(bool3(float3(7.0f, 0.0f, 0.0f).x == float3(7.0f, 0.0f, 0.0f).x, float3(7.0f, 0.0f, 0.0f).y == float3(7.0f, 0.0f, 0.0f).y, float3(7.0f, 0.0f, 0.0f).z == float3(7.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 7.0f, 0.0f).x == float3(0.0f, 7.0f, 0.0f).x, float3(0.0f, 7.0f, 0.0f).y == float3(0.0f, 7.0f, 0.0f).y, float3(0.0f, 7.0f, 0.0f).z == float3(0.0f, 7.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 7.0f).x == float3(0.0f, 0.0f, 7.0f).x, float3(0.0f, 0.0f, 7.0f).y == float3(0.0f, 0.0f, 7.0f).y, float3(0.0f, 0.0f, 7.0f).z == float3(0.0f, 0.0f, 7.0f).z))) && all(bool3(0.0f.xxx.x == 0.0f.xxx.x, 0.0f.xxx.y == 0.0f.xxx.y, 0.0f.xxx.z == 0.0f.xxx.z));
    }
    else
    {
        _144 = false;
    }
    ok = _144;
    float2x2 _148 = mul(float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f)), float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx));
    float2x2 m22 = _148;
    bool _162 = false;
    if (_144)
    {
        float2 _155 = _148[0];
        float2 _158 = _148[1];
        _162 = all(bool2(_155.x == float2(8.0f, 0.0f).x, _155.y == float2(8.0f, 0.0f).y)) && all(bool2(_158.x == float2(0.0f, 8.0f).x, _158.y == float2(0.0f, 8.0f).y));
    }
    else
    {
        _162 = false;
    }
    ok = _162;
    float3x3 _166 = mul(float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f)), float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), 0.0f.xxx));
    float3x3 m33 = _166;
    bool _185 = false;
    if (_162)
    {
        float3 _174 = _166[0];
        float3 _177 = _166[1];
        float3 _181 = _166[2];
        _185 = (all(bool3(_174.x == float3(35.0f, 0.0f, 0.0f).x, _174.y == float3(35.0f, 0.0f, 0.0f).y, _174.z == float3(35.0f, 0.0f, 0.0f).z)) && all(bool3(_177.x == float3(0.0f, 35.0f, 0.0f).x, _177.y == float3(0.0f, 35.0f, 0.0f).y, _177.z == float3(0.0f, 35.0f, 0.0f).z))) && all(bool3(_181.x == float3(0.0f, 0.0f, 35.0f).x, _181.y == float3(0.0f, 0.0f, 35.0f).y, _181.z == float3(0.0f, 0.0f, 35.0f).z));
    }
    else
    {
        _185 = false;
    }
    ok = _185;
    float3 _189 = float3(2.0f, 0.0f, 0.0f) + 1.0f.xxx;
    float3 _190 = float3(0.0f, 2.0f, 0.0f) + 1.0f.xxx;
    m23 = float2x3(_189, _190);
    bool _202 = false;
    if (_185)
    {
        _202 = all(bool3(_189.x == float3(3.0f, 1.0f, 1.0f).x, _189.y == float3(3.0f, 1.0f, 1.0f).y, _189.z == float3(3.0f, 1.0f, 1.0f).z)) && all(bool3(_190.x == float3(1.0f, 3.0f, 1.0f).x, _190.y == float3(1.0f, 3.0f, 1.0f).y, _190.z == float3(1.0f, 3.0f, 1.0f).z));
    }
    else
    {
        _202 = false;
    }
    ok = _202;
    float2 _205 = float2(4.0f, 0.0f) - 2.0f.xx;
    float2 _206 = float2(0.0f, 4.0f) - 2.0f.xx;
    float2 _207 = 0.0f.xx - 2.0f.xx;
    m32 = float3x2(_205, _206, _207);
    bool _224 = false;
    if (_202)
    {
        _224 = (all(bool2(_205.x == float2(2.0f, -2.0f).x, _205.y == float2(2.0f, -2.0f).y)) && all(bool2(_206.x == float2(-2.0f, 2.0f).x, _206.y == float2(-2.0f, 2.0f).y))) && all(bool2(_207.x == (-2.0f).xx.x, _207.y == (-2.0f).xx.y));
    }
    else
    {
        _224 = false;
    }
    ok = _224;
    float2x4 _226 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f)) * 0.25f;
    m24 = _226;
    bool _240 = false;
    if (_224)
    {
        float4 _233 = _226[0];
        float4 _236 = _226[1];
        _240 = all(bool4(_233.x == float4(0.75f, 0.0f, 0.0f, 0.0f).x, _233.y == float4(0.75f, 0.0f, 0.0f, 0.0f).y, _233.z == float4(0.75f, 0.0f, 0.0f, 0.0f).z, _233.w == float4(0.75f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_236.x == float4(0.0f, 0.75f, 0.0f, 0.0f).x, _236.y == float4(0.0f, 0.75f, 0.0f, 0.0f).y, _236.z == float4(0.0f, 0.75f, 0.0f, 0.0f).z, _236.w == float4(0.0f, 0.75f, 0.0f, 0.0f).w));
    }
    else
    {
        _240 = false;
    }
    ok = _240;
    return _240;
}

float4 main(float2 _242)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_1_m23 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
    bool _253 = false;
    if (true)
    {
        _253 = all(bool3(float3(2.0f, 0.0f, 0.0f).x == float3(2.0f, 0.0f, 0.0f).x, float3(2.0f, 0.0f, 0.0f).y == float3(2.0f, 0.0f, 0.0f).y, float3(2.0f, 0.0f, 0.0f).z == float3(2.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 2.0f, 0.0f).x == float3(0.0f, 2.0f, 0.0f).x, float3(0.0f, 2.0f, 0.0f).y == float3(0.0f, 2.0f, 0.0f).y, float3(0.0f, 2.0f, 0.0f).z == float3(0.0f, 2.0f, 0.0f).z));
    }
    else
    {
        _253 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _253;
    float2x4 _RESERVED_IDENTIFIER_FIXUP_2_m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
    bool _262 = false;
    if (_253)
    {
        _262 = all(bool4(float4(3.0f, 0.0f, 0.0f, 0.0f).x == float4(3.0f, 0.0f, 0.0f, 0.0f).x, float4(3.0f, 0.0f, 0.0f, 0.0f).y == float4(3.0f, 0.0f, 0.0f, 0.0f).y, float4(3.0f, 0.0f, 0.0f, 0.0f).z == float4(3.0f, 0.0f, 0.0f, 0.0f).z, float4(3.0f, 0.0f, 0.0f, 0.0f).w == float4(3.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 3.0f, 0.0f, 0.0f).x == float4(0.0f, 3.0f, 0.0f, 0.0f).x, float4(0.0f, 3.0f, 0.0f, 0.0f).y == float4(0.0f, 3.0f, 0.0f, 0.0f).y, float4(0.0f, 3.0f, 0.0f, 0.0f).z == float4(0.0f, 3.0f, 0.0f, 0.0f).z, float4(0.0f, 3.0f, 0.0f, 0.0f).w == float4(0.0f, 3.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _262 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _262;
    float3x2 _RESERVED_IDENTIFIER_FIXUP_3_m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
    bool _274 = false;
    if (_262)
    {
        _274 = (all(bool2(float2(4.0f, 0.0f).x == float2(4.0f, 0.0f).x, float2(4.0f, 0.0f).y == float2(4.0f, 0.0f).y)) && all(bool2(float2(0.0f, 4.0f).x == float2(0.0f, 4.0f).x, float2(0.0f, 4.0f).y == float2(0.0f, 4.0f).y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y));
    }
    else
    {
        _274 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _274;
    float2x2 _276 = mul(float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f)), float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx));
    float2x2 _RESERVED_IDENTIFIER_FIXUP_7_m22 = _276;
    bool _286 = false;
    if (_274)
    {
        float2 _279 = _276[0];
        float2 _282 = _276[1];
        _286 = all(bool2(_279.x == float2(8.0f, 0.0f).x, _279.y == float2(8.0f, 0.0f).y)) && all(bool2(_282.x == float2(0.0f, 8.0f).x, _282.y == float2(0.0f, 8.0f).y));
    }
    else
    {
        _286 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _286;
    float3 _287 = float3(2.0f, 0.0f, 0.0f) + 1.0f.xxx;
    float3 _288 = float3(0.0f, 2.0f, 0.0f) + 1.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_1_m23 = float2x3(_287, _288);
    bool _297 = false;
    if (_286)
    {
        _297 = all(bool3(_287.x == float3(3.0f, 1.0f, 1.0f).x, _287.y == float3(3.0f, 1.0f, 1.0f).y, _287.z == float3(3.0f, 1.0f, 1.0f).z)) && all(bool3(_288.x == float3(1.0f, 3.0f, 1.0f).x, _288.y == float3(1.0f, 3.0f, 1.0f).y, _288.z == float3(1.0f, 3.0f, 1.0f).z));
    }
    else
    {
        _297 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _297;
    float2 _298 = float2(4.0f, 0.0f) - 2.0f.xx;
    float2 _299 = float2(0.0f, 4.0f) - 2.0f.xx;
    float2 _300 = 0.0f.xx - 2.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_3_m32 = float3x2(_298, _299, _300);
    bool _312 = false;
    if (_297)
    {
        _312 = (all(bool2(_298.x == float2(2.0f, -2.0f).x, _298.y == float2(2.0f, -2.0f).y)) && all(bool2(_299.x == float2(-2.0f, 2.0f).x, _299.y == float2(-2.0f, 2.0f).y))) && all(bool2(_300.x == (-2.0f).xx.x, _300.y == (-2.0f).xx.y));
    }
    else
    {
        _312 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _312;
    float2x4 _313 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f)) * 0.25f;
    _RESERVED_IDENTIFIER_FIXUP_2_m24 = _313;
    bool _323 = false;
    if (_312)
    {
        float4 _316 = _313[0];
        float4 _319 = _313[1];
        _323 = all(bool4(_316.x == float4(0.75f, 0.0f, 0.0f, 0.0f).x, _316.y == float4(0.75f, 0.0f, 0.0f, 0.0f).y, _316.z == float4(0.75f, 0.0f, 0.0f, 0.0f).z, _316.w == float4(0.75f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_319.x == float4(0.0f, 0.75f, 0.0f, 0.0f).x, _319.y == float4(0.0f, 0.75f, 0.0f, 0.0f).y, _319.z == float4(0.0f, 0.75f, 0.0f, 0.0f).z, _319.w == float4(0.0f, 0.75f, 0.0f, 0.0f).w));
    }
    else
    {
        _323 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _323;
    bool _327 = false;
    if (_323)
    {
        _327 = test_half_b();
    }
    else
    {
        _327 = false;
    }
    float4 _328 = 0.0f.xxxx;
    if (_327)
    {
        _328 = _11_colorGreen;
    }
    else
    {
        _328 = _11_colorRed;
    }
    return _328;
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
