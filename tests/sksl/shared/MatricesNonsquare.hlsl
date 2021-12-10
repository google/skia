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
    float4 _227 = float4(3.0f, 0.0f, 0.0f, 0.0f) / 4.0f.xxxx;
    float4 _228 = float4(0.0f, 3.0f, 0.0f, 0.0f) / 4.0f.xxxx;
    m24 = float2x4(_227, _228);
    bool _241 = false;
    if (_224)
    {
        _241 = all(bool4(_227.x == float4(0.75f, 0.0f, 0.0f, 0.0f).x, _227.y == float4(0.75f, 0.0f, 0.0f, 0.0f).y, _227.z == float4(0.75f, 0.0f, 0.0f, 0.0f).z, _227.w == float4(0.75f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_228.x == float4(0.0f, 0.75f, 0.0f, 0.0f).x, _228.y == float4(0.0f, 0.75f, 0.0f, 0.0f).y, _228.z == float4(0.0f, 0.75f, 0.0f, 0.0f).z, _228.w == float4(0.0f, 0.75f, 0.0f, 0.0f).w));
    }
    else
    {
        _241 = false;
    }
    ok = _241;
    return _241;
}

float4 main(float2 _243)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_1_m23 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
    bool _254 = false;
    if (true)
    {
        _254 = all(bool3(float3(2.0f, 0.0f, 0.0f).x == float3(2.0f, 0.0f, 0.0f).x, float3(2.0f, 0.0f, 0.0f).y == float3(2.0f, 0.0f, 0.0f).y, float3(2.0f, 0.0f, 0.0f).z == float3(2.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 2.0f, 0.0f).x == float3(0.0f, 2.0f, 0.0f).x, float3(0.0f, 2.0f, 0.0f).y == float3(0.0f, 2.0f, 0.0f).y, float3(0.0f, 2.0f, 0.0f).z == float3(0.0f, 2.0f, 0.0f).z));
    }
    else
    {
        _254 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _254;
    float2x4 _RESERVED_IDENTIFIER_FIXUP_2_m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
    bool _263 = false;
    if (_254)
    {
        _263 = all(bool4(float4(3.0f, 0.0f, 0.0f, 0.0f).x == float4(3.0f, 0.0f, 0.0f, 0.0f).x, float4(3.0f, 0.0f, 0.0f, 0.0f).y == float4(3.0f, 0.0f, 0.0f, 0.0f).y, float4(3.0f, 0.0f, 0.0f, 0.0f).z == float4(3.0f, 0.0f, 0.0f, 0.0f).z, float4(3.0f, 0.0f, 0.0f, 0.0f).w == float4(3.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 3.0f, 0.0f, 0.0f).x == float4(0.0f, 3.0f, 0.0f, 0.0f).x, float4(0.0f, 3.0f, 0.0f, 0.0f).y == float4(0.0f, 3.0f, 0.0f, 0.0f).y, float4(0.0f, 3.0f, 0.0f, 0.0f).z == float4(0.0f, 3.0f, 0.0f, 0.0f).z, float4(0.0f, 3.0f, 0.0f, 0.0f).w == float4(0.0f, 3.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _263 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _263;
    float3x2 _RESERVED_IDENTIFIER_FIXUP_3_m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
    bool _275 = false;
    if (_263)
    {
        _275 = (all(bool2(float2(4.0f, 0.0f).x == float2(4.0f, 0.0f).x, float2(4.0f, 0.0f).y == float2(4.0f, 0.0f).y)) && all(bool2(float2(0.0f, 4.0f).x == float2(0.0f, 4.0f).x, float2(0.0f, 4.0f).y == float2(0.0f, 4.0f).y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y));
    }
    else
    {
        _275 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _275;
    float2x2 _277 = mul(float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f)), float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx));
    float2x2 _RESERVED_IDENTIFIER_FIXUP_7_m22 = _277;
    bool _287 = false;
    if (_275)
    {
        float2 _280 = _277[0];
        float2 _283 = _277[1];
        _287 = all(bool2(_280.x == float2(8.0f, 0.0f).x, _280.y == float2(8.0f, 0.0f).y)) && all(bool2(_283.x == float2(0.0f, 8.0f).x, _283.y == float2(0.0f, 8.0f).y));
    }
    else
    {
        _287 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _287;
    float3 _288 = float3(2.0f, 0.0f, 0.0f) + 1.0f.xxx;
    float3 _289 = float3(0.0f, 2.0f, 0.0f) + 1.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_1_m23 = float2x3(_288, _289);
    bool _298 = false;
    if (_287)
    {
        _298 = all(bool3(_288.x == float3(3.0f, 1.0f, 1.0f).x, _288.y == float3(3.0f, 1.0f, 1.0f).y, _288.z == float3(3.0f, 1.0f, 1.0f).z)) && all(bool3(_289.x == float3(1.0f, 3.0f, 1.0f).x, _289.y == float3(1.0f, 3.0f, 1.0f).y, _289.z == float3(1.0f, 3.0f, 1.0f).z));
    }
    else
    {
        _298 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _298;
    float2 _299 = float2(4.0f, 0.0f) - 2.0f.xx;
    float2 _300 = float2(0.0f, 4.0f) - 2.0f.xx;
    float2 _301 = 0.0f.xx - 2.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_3_m32 = float3x2(_299, _300, _301);
    bool _313 = false;
    if (_298)
    {
        _313 = (all(bool2(_299.x == float2(2.0f, -2.0f).x, _299.y == float2(2.0f, -2.0f).y)) && all(bool2(_300.x == float2(-2.0f, 2.0f).x, _300.y == float2(-2.0f, 2.0f).y))) && all(bool2(_301.x == (-2.0f).xx.x, _301.y == (-2.0f).xx.y));
    }
    else
    {
        _313 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _313;
    float4 _314 = float4(3.0f, 0.0f, 0.0f, 0.0f) / 4.0f.xxxx;
    float4 _315 = float4(0.0f, 3.0f, 0.0f, 0.0f) / 4.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_2_m24 = float2x4(_314, _315);
    bool _324 = false;
    if (_313)
    {
        _324 = all(bool4(_314.x == float4(0.75f, 0.0f, 0.0f, 0.0f).x, _314.y == float4(0.75f, 0.0f, 0.0f, 0.0f).y, _314.z == float4(0.75f, 0.0f, 0.0f, 0.0f).z, _314.w == float4(0.75f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_315.x == float4(0.0f, 0.75f, 0.0f, 0.0f).x, _315.y == float4(0.0f, 0.75f, 0.0f, 0.0f).y, _315.z == float4(0.0f, 0.75f, 0.0f, 0.0f).z, _315.w == float4(0.0f, 0.75f, 0.0f, 0.0f).w));
    }
    else
    {
        _324 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _324;
    bool _328 = false;
    if (_324)
    {
        _328 = test_half_b();
    }
    else
    {
        _328 = false;
    }
    float4 _329 = 0.0f.xxxx;
    if (_328)
    {
        _329 = _11_colorGreen;
    }
    else
    {
        _329 = _11_colorRed;
    }
    return _329;
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
