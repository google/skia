cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorGreen : packoffset(c0);
    float4 _8_colorRed : packoffset(c1);
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
    bool _44 = false;
    if (true)
    {
        _44 = all(bool3(float3(2.0f, 0.0f, 0.0f).x == float3(2.0f, 0.0f, 0.0f).x, float3(2.0f, 0.0f, 0.0f).y == float3(2.0f, 0.0f, 0.0f).y, float3(2.0f, 0.0f, 0.0f).z == float3(2.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 2.0f, 0.0f).x == float3(0.0f, 2.0f, 0.0f).x, float3(0.0f, 2.0f, 0.0f).y == float3(0.0f, 2.0f, 0.0f).y, float3(0.0f, 2.0f, 0.0f).z == float3(0.0f, 2.0f, 0.0f).z));
    }
    else
    {
        _44 = false;
    }
    ok = _44;
    float2x4 m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
    bool _60 = false;
    if (_44)
    {
        _60 = all(bool4(float4(3.0f, 0.0f, 0.0f, 0.0f).x == float4(3.0f, 0.0f, 0.0f, 0.0f).x, float4(3.0f, 0.0f, 0.0f, 0.0f).y == float4(3.0f, 0.0f, 0.0f, 0.0f).y, float4(3.0f, 0.0f, 0.0f, 0.0f).z == float4(3.0f, 0.0f, 0.0f, 0.0f).z, float4(3.0f, 0.0f, 0.0f, 0.0f).w == float4(3.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 3.0f, 0.0f, 0.0f).x == float4(0.0f, 3.0f, 0.0f, 0.0f).x, float4(0.0f, 3.0f, 0.0f, 0.0f).y == float4(0.0f, 3.0f, 0.0f, 0.0f).y, float4(0.0f, 3.0f, 0.0f, 0.0f).z == float4(0.0f, 3.0f, 0.0f, 0.0f).z, float4(0.0f, 3.0f, 0.0f, 0.0f).w == float4(0.0f, 3.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _60 = false;
    }
    ok = _60;
    float3x2 m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
    bool _79 = false;
    if (_60)
    {
        _79 = (all(bool2(float2(4.0f, 0.0f).x == float2(4.0f, 0.0f).x, float2(4.0f, 0.0f).y == float2(4.0f, 0.0f).y)) && all(bool2(float2(0.0f, 4.0f).x == float2(0.0f, 4.0f).x, float2(0.0f, 4.0f).y == float2(0.0f, 4.0f).y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y));
    }
    else
    {
        _79 = false;
    }
    ok = _79;
    float3x4 m34 = float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f));
    bool _98 = false;
    if (_79)
    {
        _98 = (all(bool4(float4(5.0f, 0.0f, 0.0f, 0.0f).x == float4(5.0f, 0.0f, 0.0f, 0.0f).x, float4(5.0f, 0.0f, 0.0f, 0.0f).y == float4(5.0f, 0.0f, 0.0f, 0.0f).y, float4(5.0f, 0.0f, 0.0f, 0.0f).z == float4(5.0f, 0.0f, 0.0f, 0.0f).z, float4(5.0f, 0.0f, 0.0f, 0.0f).w == float4(5.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 5.0f, 0.0f, 0.0f).x == float4(0.0f, 5.0f, 0.0f, 0.0f).x, float4(0.0f, 5.0f, 0.0f, 0.0f).y == float4(0.0f, 5.0f, 0.0f, 0.0f).y, float4(0.0f, 5.0f, 0.0f, 0.0f).z == float4(0.0f, 5.0f, 0.0f, 0.0f).z, float4(0.0f, 5.0f, 0.0f, 0.0f).w == float4(0.0f, 5.0f, 0.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 5.0f, 0.0f).x == float4(0.0f, 0.0f, 5.0f, 0.0f).x, float4(0.0f, 0.0f, 5.0f, 0.0f).y == float4(0.0f, 0.0f, 5.0f, 0.0f).y, float4(0.0f, 0.0f, 5.0f, 0.0f).z == float4(0.0f, 0.0f, 5.0f, 0.0f).z, float4(0.0f, 0.0f, 5.0f, 0.0f).w == float4(0.0f, 0.0f, 5.0f, 0.0f).w));
    }
    else
    {
        _98 = false;
    }
    ok = _98;
    float4x2 m42 = float4x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f), 0.0f.xx, 0.0f.xx);
    bool _119 = false;
    if (_98)
    {
        _119 = ((all(bool2(float2(6.0f, 0.0f).x == float2(6.0f, 0.0f).x, float2(6.0f, 0.0f).y == float2(6.0f, 0.0f).y)) && all(bool2(float2(0.0f, 6.0f).x == float2(0.0f, 6.0f).x, float2(0.0f, 6.0f).y == float2(0.0f, 6.0f).y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y));
    }
    else
    {
        _119 = false;
    }
    ok = _119;
    float4x3 m43 = float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), 0.0f.xxx);
    bool _142 = false;
    if (_119)
    {
        _142 = ((all(bool3(float3(7.0f, 0.0f, 0.0f).x == float3(7.0f, 0.0f, 0.0f).x, float3(7.0f, 0.0f, 0.0f).y == float3(7.0f, 0.0f, 0.0f).y, float3(7.0f, 0.0f, 0.0f).z == float3(7.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 7.0f, 0.0f).x == float3(0.0f, 7.0f, 0.0f).x, float3(0.0f, 7.0f, 0.0f).y == float3(0.0f, 7.0f, 0.0f).y, float3(0.0f, 7.0f, 0.0f).z == float3(0.0f, 7.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 7.0f).x == float3(0.0f, 0.0f, 7.0f).x, float3(0.0f, 0.0f, 7.0f).y == float3(0.0f, 0.0f, 7.0f).y, float3(0.0f, 0.0f, 7.0f).z == float3(0.0f, 0.0f, 7.0f).z))) && all(bool3(0.0f.xxx.x == 0.0f.xxx.x, 0.0f.xxx.y == 0.0f.xxx.y, 0.0f.xxx.z == 0.0f.xxx.z));
    }
    else
    {
        _142 = false;
    }
    ok = _142;
    float2x2 _146 = mul(float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f)), float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx));
    float2x2 m22 = _146;
    bool _160 = false;
    if (_142)
    {
        float2 _153 = _146[0];
        float2 _156 = _146[1];
        _160 = all(bool2(_153.x == float2(8.0f, 0.0f).x, _153.y == float2(8.0f, 0.0f).y)) && all(bool2(_156.x == float2(0.0f, 8.0f).x, _156.y == float2(0.0f, 8.0f).y));
    }
    else
    {
        _160 = false;
    }
    ok = _160;
    float3x3 _164 = mul(float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f)), float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), 0.0f.xxx));
    float3x3 m33 = _164;
    bool _183 = false;
    if (_160)
    {
        float3 _172 = _164[0];
        float3 _175 = _164[1];
        float3 _179 = _164[2];
        _183 = (all(bool3(_172.x == float3(35.0f, 0.0f, 0.0f).x, _172.y == float3(35.0f, 0.0f, 0.0f).y, _172.z == float3(35.0f, 0.0f, 0.0f).z)) && all(bool3(_175.x == float3(0.0f, 35.0f, 0.0f).x, _175.y == float3(0.0f, 35.0f, 0.0f).y, _175.z == float3(0.0f, 35.0f, 0.0f).z))) && all(bool3(_179.x == float3(0.0f, 0.0f, 35.0f).x, _179.y == float3(0.0f, 0.0f, 35.0f).y, _179.z == float3(0.0f, 0.0f, 35.0f).z));
    }
    else
    {
        _183 = false;
    }
    ok = _183;
    float3 _187 = float3(2.0f, 0.0f, 0.0f) + 1.0f.xxx;
    float3 _188 = float3(0.0f, 2.0f, 0.0f) + 1.0f.xxx;
    m23 = float2x3(_187, _188);
    bool _200 = false;
    if (_183)
    {
        _200 = all(bool3(_187.x == float3(3.0f, 1.0f, 1.0f).x, _187.y == float3(3.0f, 1.0f, 1.0f).y, _187.z == float3(3.0f, 1.0f, 1.0f).z)) && all(bool3(_188.x == float3(1.0f, 3.0f, 1.0f).x, _188.y == float3(1.0f, 3.0f, 1.0f).y, _188.z == float3(1.0f, 3.0f, 1.0f).z));
    }
    else
    {
        _200 = false;
    }
    ok = _200;
    float2 _203 = float2(4.0f, 0.0f) - 2.0f.xx;
    float2 _204 = float2(0.0f, 4.0f) - 2.0f.xx;
    float2 _205 = 0.0f.xx - 2.0f.xx;
    m32 = float3x2(_203, _204, _205);
    bool _222 = false;
    if (_200)
    {
        _222 = (all(bool2(_203.x == float2(2.0f, -2.0f).x, _203.y == float2(2.0f, -2.0f).y)) && all(bool2(_204.x == float2(-2.0f, 2.0f).x, _204.y == float2(-2.0f, 2.0f).y))) && all(bool2(_205.x == (-2.0f).xx.x, _205.y == (-2.0f).xx.y));
    }
    else
    {
        _222 = false;
    }
    ok = _222;
    float2x4 _224 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f)) * 0.25f;
    m24 = _224;
    bool _238 = false;
    if (_222)
    {
        float4 _231 = _224[0];
        float4 _234 = _224[1];
        _238 = all(bool4(_231.x == float4(0.75f, 0.0f, 0.0f, 0.0f).x, _231.y == float4(0.75f, 0.0f, 0.0f, 0.0f).y, _231.z == float4(0.75f, 0.0f, 0.0f, 0.0f).z, _231.w == float4(0.75f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_234.x == float4(0.0f, 0.75f, 0.0f, 0.0f).x, _234.y == float4(0.0f, 0.75f, 0.0f, 0.0f).y, _234.z == float4(0.0f, 0.75f, 0.0f, 0.0f).z, _234.w == float4(0.0f, 0.75f, 0.0f, 0.0f).w));
    }
    else
    {
        _238 = false;
    }
    ok = _238;
    return _238;
}

float4 main(float2 _240)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_1_m23 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
    bool _251 = false;
    if (true)
    {
        _251 = all(bool3(float3(2.0f, 0.0f, 0.0f).x == float3(2.0f, 0.0f, 0.0f).x, float3(2.0f, 0.0f, 0.0f).y == float3(2.0f, 0.0f, 0.0f).y, float3(2.0f, 0.0f, 0.0f).z == float3(2.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 2.0f, 0.0f).x == float3(0.0f, 2.0f, 0.0f).x, float3(0.0f, 2.0f, 0.0f).y == float3(0.0f, 2.0f, 0.0f).y, float3(0.0f, 2.0f, 0.0f).z == float3(0.0f, 2.0f, 0.0f).z));
    }
    else
    {
        _251 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _251;
    float2x4 _RESERVED_IDENTIFIER_FIXUP_2_m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
    bool _260 = false;
    if (_251)
    {
        _260 = all(bool4(float4(3.0f, 0.0f, 0.0f, 0.0f).x == float4(3.0f, 0.0f, 0.0f, 0.0f).x, float4(3.0f, 0.0f, 0.0f, 0.0f).y == float4(3.0f, 0.0f, 0.0f, 0.0f).y, float4(3.0f, 0.0f, 0.0f, 0.0f).z == float4(3.0f, 0.0f, 0.0f, 0.0f).z, float4(3.0f, 0.0f, 0.0f, 0.0f).w == float4(3.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 3.0f, 0.0f, 0.0f).x == float4(0.0f, 3.0f, 0.0f, 0.0f).x, float4(0.0f, 3.0f, 0.0f, 0.0f).y == float4(0.0f, 3.0f, 0.0f, 0.0f).y, float4(0.0f, 3.0f, 0.0f, 0.0f).z == float4(0.0f, 3.0f, 0.0f, 0.0f).z, float4(0.0f, 3.0f, 0.0f, 0.0f).w == float4(0.0f, 3.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _260 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _260;
    float3x2 _RESERVED_IDENTIFIER_FIXUP_3_m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
    bool _272 = false;
    if (_260)
    {
        _272 = (all(bool2(float2(4.0f, 0.0f).x == float2(4.0f, 0.0f).x, float2(4.0f, 0.0f).y == float2(4.0f, 0.0f).y)) && all(bool2(float2(0.0f, 4.0f).x == float2(0.0f, 4.0f).x, float2(0.0f, 4.0f).y == float2(0.0f, 4.0f).y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y));
    }
    else
    {
        _272 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _272;
    float2x2 _274 = mul(float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f)), float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx));
    float2x2 _RESERVED_IDENTIFIER_FIXUP_7_m22 = _274;
    bool _284 = false;
    if (_272)
    {
        float2 _277 = _274[0];
        float2 _280 = _274[1];
        _284 = all(bool2(_277.x == float2(8.0f, 0.0f).x, _277.y == float2(8.0f, 0.0f).y)) && all(bool2(_280.x == float2(0.0f, 8.0f).x, _280.y == float2(0.0f, 8.0f).y));
    }
    else
    {
        _284 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _284;
    float3 _285 = float3(2.0f, 0.0f, 0.0f) + 1.0f.xxx;
    float3 _286 = float3(0.0f, 2.0f, 0.0f) + 1.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_1_m23 = float2x3(_285, _286);
    bool _295 = false;
    if (_284)
    {
        _295 = all(bool3(_285.x == float3(3.0f, 1.0f, 1.0f).x, _285.y == float3(3.0f, 1.0f, 1.0f).y, _285.z == float3(3.0f, 1.0f, 1.0f).z)) && all(bool3(_286.x == float3(1.0f, 3.0f, 1.0f).x, _286.y == float3(1.0f, 3.0f, 1.0f).y, _286.z == float3(1.0f, 3.0f, 1.0f).z));
    }
    else
    {
        _295 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _295;
    float2 _296 = float2(4.0f, 0.0f) - 2.0f.xx;
    float2 _297 = float2(0.0f, 4.0f) - 2.0f.xx;
    float2 _298 = 0.0f.xx - 2.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_3_m32 = float3x2(_296, _297, _298);
    bool _310 = false;
    if (_295)
    {
        _310 = (all(bool2(_296.x == float2(2.0f, -2.0f).x, _296.y == float2(2.0f, -2.0f).y)) && all(bool2(_297.x == float2(-2.0f, 2.0f).x, _297.y == float2(-2.0f, 2.0f).y))) && all(bool2(_298.x == (-2.0f).xx.x, _298.y == (-2.0f).xx.y));
    }
    else
    {
        _310 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _310;
    float2x4 _311 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f)) * 0.25f;
    _RESERVED_IDENTIFIER_FIXUP_2_m24 = _311;
    bool _321 = false;
    if (_310)
    {
        float4 _314 = _311[0];
        float4 _317 = _311[1];
        _321 = all(bool4(_314.x == float4(0.75f, 0.0f, 0.0f, 0.0f).x, _314.y == float4(0.75f, 0.0f, 0.0f, 0.0f).y, _314.z == float4(0.75f, 0.0f, 0.0f, 0.0f).z, _314.w == float4(0.75f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_317.x == float4(0.0f, 0.75f, 0.0f, 0.0f).x, _317.y == float4(0.0f, 0.75f, 0.0f, 0.0f).y, _317.z == float4(0.0f, 0.75f, 0.0f, 0.0f).z, _317.w == float4(0.0f, 0.75f, 0.0f, 0.0f).w));
    }
    else
    {
        _321 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _321;
    bool _325 = false;
    if (_321)
    {
        _325 = test_half_b();
    }
    else
    {
        _325 = false;
    }
    float4 _326 = 0.0f.xxxx;
    if (_325)
    {
        _326 = _8_colorGreen;
    }
    else
    {
        _326 = _8_colorRed;
    }
    return _326;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
