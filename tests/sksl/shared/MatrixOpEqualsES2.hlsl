cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorRed : packoffset(c0);
    float4 _8_colorGreen : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_matrix_op_matrix_half_b()
{
    bool ok = true;
    float3x3 splat_4 = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3x3 splat_2 = float3x3(2.0f.xxx, 2.0f.xxx, 2.0f.xxx);
    float3x3 m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _43 = float3(2.0f, 0.0f, 0.0f) + 4.0f.xxx;
    float3 _44 = float3(0.0f, 2.0f, 0.0f) + 4.0f.xxx;
    float3 _45 = float3(0.0f, 0.0f, 2.0f) + 4.0f.xxx;
    m = float3x3(_43, _44, _45);
    bool _64 = false;
    if (true)
    {
        _64 = (all(bool3(_43.x == float3(6.0f, 4.0f, 4.0f).x, _43.y == float3(6.0f, 4.0f, 4.0f).y, _43.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_44.x == float3(4.0f, 6.0f, 4.0f).x, _44.y == float3(4.0f, 6.0f, 4.0f).y, _44.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_45.x == float3(4.0f, 4.0f, 6.0f).x, _45.y == float3(4.0f, 4.0f, 6.0f).y, _45.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _64 = false;
    }
    ok = _64;
    m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _65 = float3(2.0f, 0.0f, 0.0f) - 4.0f.xxx;
    float3 _66 = float3(0.0f, 2.0f, 0.0f) - 4.0f.xxx;
    float3 _67 = float3(0.0f, 0.0f, 2.0f) - 4.0f.xxx;
    m = float3x3(_65, _66, _67);
    bool _85 = false;
    if (_64)
    {
        _85 = (all(bool3(_65.x == float3(-2.0f, -4.0f, -4.0f).x, _65.y == float3(-2.0f, -4.0f, -4.0f).y, _65.z == float3(-2.0f, -4.0f, -4.0f).z)) && all(bool3(_66.x == float3(-4.0f, -2.0f, -4.0f).x, _66.y == float3(-4.0f, -2.0f, -4.0f).y, _66.z == float3(-4.0f, -2.0f, -4.0f).z))) && all(bool3(_67.x == float3(-4.0f, -4.0f, -2.0f).x, _67.y == float3(-4.0f, -4.0f, -2.0f).y, _67.z == float3(-4.0f, -4.0f, -2.0f).z));
    }
    else
    {
        _85 = false;
    }
    ok = _85;
    m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _86 = float3(2.0f, 0.0f, 0.0f) / 4.0f.xxx;
    float3 _87 = float3(0.0f, 2.0f, 0.0f) / 4.0f.xxx;
    float3 _88 = float3(0.0f, 0.0f, 2.0f) / 4.0f.xxx;
    m = float3x3(_86, _87, _88);
    bool _105 = false;
    if (_85)
    {
        _105 = (all(bool3(_86.x == float3(0.5f, 0.0f, 0.0f).x, _86.y == float3(0.5f, 0.0f, 0.0f).y, _86.z == float3(0.5f, 0.0f, 0.0f).z)) && all(bool3(_87.x == float3(0.0f, 0.5f, 0.0f).x, _87.y == float3(0.0f, 0.5f, 0.0f).y, _87.z == float3(0.0f, 0.5f, 0.0f).z))) && all(bool3(_88.x == float3(0.0f, 0.0f, 0.5f).x, _88.y == float3(0.0f, 0.0f, 0.5f).y, _88.z == float3(0.0f, 0.0f, 0.5f).z));
    }
    else
    {
        _105 = false;
    }
    ok = _105;
    m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _106 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _107 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    float3 _108 = 4.0f.xxx + float3(0.0f, 0.0f, 2.0f);
    m = float3x3(_106, _107, _108);
    bool _120 = false;
    if (_105)
    {
        _120 = (all(bool3(_106.x == float3(6.0f, 4.0f, 4.0f).x, _106.y == float3(6.0f, 4.0f, 4.0f).y, _106.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_107.x == float3(4.0f, 6.0f, 4.0f).x, _107.y == float3(4.0f, 6.0f, 4.0f).y, _107.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_108.x == float3(4.0f, 4.0f, 6.0f).x, _108.y == float3(4.0f, 4.0f, 6.0f).y, _108.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _120 = false;
    }
    ok = _120;
    m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _121 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _122 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    float3 _123 = 4.0f.xxx - float3(0.0f, 0.0f, 2.0f);
    m = float3x3(_121, _122, _123);
    bool _139 = false;
    if (_120)
    {
        _139 = (all(bool3(_121.x == float3(2.0f, 4.0f, 4.0f).x, _121.y == float3(2.0f, 4.0f, 4.0f).y, _121.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_122.x == float3(4.0f, 2.0f, 4.0f).x, _122.y == float3(4.0f, 2.0f, 4.0f).y, _122.z == float3(4.0f, 2.0f, 4.0f).z))) && all(bool3(_123.x == float3(4.0f, 4.0f, 2.0f).x, _123.y == float3(4.0f, 4.0f, 2.0f).y, _123.z == float3(4.0f, 4.0f, 2.0f).z));
    }
    else
    {
        _139 = false;
    }
    ok = _139;
    m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _140 = 4.0f.xxx / 2.0f.xxx;
    float3 _141 = 4.0f.xxx / 2.0f.xxx;
    float3 _142 = 4.0f.xxx / 2.0f.xxx;
    m = float3x3(_140, _141, _142);
    bool _154 = false;
    if (_139)
    {
        _154 = (all(bool3(_140.x == 2.0f.xxx.x, _140.y == 2.0f.xxx.y, _140.z == 2.0f.xxx.z)) && all(bool3(_141.x == 2.0f.xxx.x, _141.y == 2.0f.xxx.y, _141.z == 2.0f.xxx.z))) && all(bool3(_142.x == 2.0f.xxx.x, _142.y == 2.0f.xxx.y, _142.z == 2.0f.xxx.z));
    }
    else
    {
        _154 = false;
    }
    ok = _154;
    float4x4 m_1 = float4x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f), float4(13.0f, 14.0f, 15.0f, 16.0f));
    float4 _181 = float4(1.0f, 2.0f, 3.0f, 4.0f) + float4(16.0f, 15.0f, 14.0f, 13.0f);
    float4 _182 = float4(5.0f, 6.0f, 7.0f, 8.0f) + float4(12.0f, 11.0f, 10.0f, 9.0f);
    float4 _183 = float4(9.0f, 10.0f, 11.0f, 12.0f) + float4(8.0f, 7.0f, 6.0f, 5.0f);
    float4 _184 = float4(13.0f, 14.0f, 15.0f, 16.0f) + float4(4.0f, 3.0f, 2.0f, 1.0f);
    m_1 = float4x4(_181, _182, _183, _184);
    bool _203 = false;
    if (_154)
    {
        _203 = ((all(bool4(_181.x == 17.0f.xxxx.x, _181.y == 17.0f.xxxx.y, _181.z == 17.0f.xxxx.z, _181.w == 17.0f.xxxx.w)) && all(bool4(_182.x == 17.0f.xxxx.x, _182.y == 17.0f.xxxx.y, _182.z == 17.0f.xxxx.z, _182.w == 17.0f.xxxx.w))) && all(bool4(_183.x == 17.0f.xxxx.x, _183.y == 17.0f.xxxx.y, _183.z == 17.0f.xxxx.z, _183.w == 17.0f.xxxx.w))) && all(bool4(_184.x == 17.0f.xxxx.x, _184.y == 17.0f.xxxx.y, _184.z == 17.0f.xxxx.z, _184.w == 17.0f.xxxx.w));
    }
    else
    {
        _203 = false;
    }
    ok = _203;
    float2x2 m_2 = float2x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f));
    float2 _216 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _217 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    m_2 = float2x2(_216, _217);
    bool _233 = false;
    if (_203)
    {
        _233 = all(bool2(_216.x == float2(9.0f, 18.0f).x, _216.y == float2(9.0f, 18.0f).y)) && all(bool2(_217.x == float2(27.0f, 36.0f).x, _217.y == float2(27.0f, 36.0f).y));
    }
    else
    {
        _233 = false;
    }
    ok = _233;
    float2x2 m_3 = float2x2(float2(2.0f, 4.0f), float2(6.0f, 8.0f));
    float2 _240 = float2(2.0f, 4.0f) / 2.0f.xx;
    float2 _241 = float2(6.0f, 8.0f) / float2(2.0f, 4.0f);
    m_3 = float2x2(_240, _241);
    bool _252 = false;
    if (_233)
    {
        _252 = all(bool2(_240.x == float2(1.0f, 2.0f).x, _240.y == float2(1.0f, 2.0f).y)) && all(bool2(_241.x == float2(3.0f, 2.0f).x, _241.y == float2(3.0f, 2.0f).y));
    }
    else
    {
        _252 = false;
    }
    ok = _252;
    float2x2 m_4 = float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f));
    float2x2 _258 = mul(float2x2(float2(3.0f, 5.0f), float2(3.0f, 2.0f)), float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f)));
    m_4 = _258;
    bool _273 = false;
    if (_252)
    {
        float2 _266 = _258[0];
        float2 _269 = _258[1];
        _273 = all(bool2(_266.x == float2(38.0f, 26.0f).x, _266.y == float2(38.0f, 26.0f).y)) && all(bool2(_269.x == float2(17.0f, 14.0f).x, _269.y == float2(17.0f, 14.0f).y));
    }
    else
    {
        _273 = false;
    }
    ok = _273;
    float3x3 m_5 = float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f));
    float3x3 _283 = mul(float3x3(float3(3.0f, 3.0f, 4.0f), float3(2.0f, 3.0f, 4.0f), float3(4.0f, 9.0f, 2.0f)), float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f)));
    m_5 = _283;
    bool _310 = false;
    if (_273)
    {
        float3 _299 = _283[0];
        float3 _302 = _283[1];
        float3 _306 = _283[2];
        _310 = (all(bool3(_299.x == float3(130.0f, 51.0f, 35.0f).x, _299.y == float3(130.0f, 51.0f, 35.0f).y, _299.z == float3(130.0f, 51.0f, 35.0f).z)) && all(bool3(_302.x == float3(120.0f, 47.0f, 33.0f).x, _302.y == float3(120.0f, 47.0f, 33.0f).y, _302.z == float3(120.0f, 47.0f, 33.0f).z))) && all(bool3(_306.x == float3(240.0f, 73.0f, 45.0f).x, _306.y == float3(240.0f, 73.0f, 45.0f).y, _306.z == float3(240.0f, 73.0f, 45.0f).z));
    }
    else
    {
        _310 = false;
    }
    ok = _310;
    return _310;
}

float4 main(float2 _312)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_1_splat_4 = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3x3 _RESERVED_IDENTIFIER_FIXUP_2_splat_2 = float3x3(2.0f.xxx, 2.0f.xxx, 2.0f.xxx);
    float3x3 _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _318 = float3(2.0f, 0.0f, 0.0f) + 4.0f.xxx;
    float3 _319 = float3(0.0f, 2.0f, 0.0f) + 4.0f.xxx;
    float3 _320 = float3(0.0f, 0.0f, 2.0f) + 4.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_318, _319, _320);
    bool _332 = false;
    if (true)
    {
        _332 = (all(bool3(_318.x == float3(6.0f, 4.0f, 4.0f).x, _318.y == float3(6.0f, 4.0f, 4.0f).y, _318.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_319.x == float3(4.0f, 6.0f, 4.0f).x, _319.y == float3(4.0f, 6.0f, 4.0f).y, _319.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_320.x == float3(4.0f, 4.0f, 6.0f).x, _320.y == float3(4.0f, 4.0f, 6.0f).y, _320.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _332 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _332;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _333 = float3(2.0f, 0.0f, 0.0f) - 4.0f.xxx;
    float3 _334 = float3(0.0f, 2.0f, 0.0f) - 4.0f.xxx;
    float3 _335 = float3(0.0f, 0.0f, 2.0f) - 4.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_333, _334, _335);
    bool _347 = false;
    if (_332)
    {
        _347 = (all(bool3(_333.x == float3(-2.0f, -4.0f, -4.0f).x, _333.y == float3(-2.0f, -4.0f, -4.0f).y, _333.z == float3(-2.0f, -4.0f, -4.0f).z)) && all(bool3(_334.x == float3(-4.0f, -2.0f, -4.0f).x, _334.y == float3(-4.0f, -2.0f, -4.0f).y, _334.z == float3(-4.0f, -2.0f, -4.0f).z))) && all(bool3(_335.x == float3(-4.0f, -4.0f, -2.0f).x, _335.y == float3(-4.0f, -4.0f, -2.0f).y, _335.z == float3(-4.0f, -4.0f, -2.0f).z));
    }
    else
    {
        _347 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _347;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _348 = float3(2.0f, 0.0f, 0.0f) / 4.0f.xxx;
    float3 _349 = float3(0.0f, 2.0f, 0.0f) / 4.0f.xxx;
    float3 _350 = float3(0.0f, 0.0f, 2.0f) / 4.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_348, _349, _350);
    bool _362 = false;
    if (_347)
    {
        _362 = (all(bool3(_348.x == float3(0.5f, 0.0f, 0.0f).x, _348.y == float3(0.5f, 0.0f, 0.0f).y, _348.z == float3(0.5f, 0.0f, 0.0f).z)) && all(bool3(_349.x == float3(0.0f, 0.5f, 0.0f).x, _349.y == float3(0.0f, 0.5f, 0.0f).y, _349.z == float3(0.0f, 0.5f, 0.0f).z))) && all(bool3(_350.x == float3(0.0f, 0.0f, 0.5f).x, _350.y == float3(0.0f, 0.0f, 0.5f).y, _350.z == float3(0.0f, 0.0f, 0.5f).z));
    }
    else
    {
        _362 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _362;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _363 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _364 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    float3 _365 = 4.0f.xxx + float3(0.0f, 0.0f, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_363, _364, _365);
    bool _377 = false;
    if (_362)
    {
        _377 = (all(bool3(_363.x == float3(6.0f, 4.0f, 4.0f).x, _363.y == float3(6.0f, 4.0f, 4.0f).y, _363.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_364.x == float3(4.0f, 6.0f, 4.0f).x, _364.y == float3(4.0f, 6.0f, 4.0f).y, _364.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_365.x == float3(4.0f, 4.0f, 6.0f).x, _365.y == float3(4.0f, 4.0f, 6.0f).y, _365.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _377 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _377;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _378 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _379 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    float3 _380 = 4.0f.xxx - float3(0.0f, 0.0f, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_378, _379, _380);
    bool _392 = false;
    if (_377)
    {
        _392 = (all(bool3(_378.x == float3(2.0f, 4.0f, 4.0f).x, _378.y == float3(2.0f, 4.0f, 4.0f).y, _378.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_379.x == float3(4.0f, 2.0f, 4.0f).x, _379.y == float3(4.0f, 2.0f, 4.0f).y, _379.z == float3(4.0f, 2.0f, 4.0f).z))) && all(bool3(_380.x == float3(4.0f, 4.0f, 2.0f).x, _380.y == float3(4.0f, 4.0f, 2.0f).y, _380.z == float3(4.0f, 4.0f, 2.0f).z));
    }
    else
    {
        _392 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _392;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _393 = 4.0f.xxx / 2.0f.xxx;
    float3 _394 = 4.0f.xxx / 2.0f.xxx;
    float3 _395 = 4.0f.xxx / 2.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_393, _394, _395);
    bool _407 = false;
    if (_392)
    {
        _407 = (all(bool3(_393.x == 2.0f.xxx.x, _393.y == 2.0f.xxx.y, _393.z == 2.0f.xxx.z)) && all(bool3(_394.x == 2.0f.xxx.x, _394.y == 2.0f.xxx.y, _394.z == 2.0f.xxx.z))) && all(bool3(_395.x == 2.0f.xxx.x, _395.y == 2.0f.xxx.y, _395.z == 2.0f.xxx.z));
    }
    else
    {
        _407 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _407;
    float4x4 _RESERVED_IDENTIFIER_FIXUP_4_m = float4x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f), float4(13.0f, 14.0f, 15.0f, 16.0f));
    float4 _409 = float4(1.0f, 2.0f, 3.0f, 4.0f) + float4(16.0f, 15.0f, 14.0f, 13.0f);
    float4 _410 = float4(5.0f, 6.0f, 7.0f, 8.0f) + float4(12.0f, 11.0f, 10.0f, 9.0f);
    float4 _411 = float4(9.0f, 10.0f, 11.0f, 12.0f) + float4(8.0f, 7.0f, 6.0f, 5.0f);
    float4 _412 = float4(13.0f, 14.0f, 15.0f, 16.0f) + float4(4.0f, 3.0f, 2.0f, 1.0f);
    _RESERVED_IDENTIFIER_FIXUP_4_m = float4x4(_409, _410, _411, _412);
    bool _427 = false;
    if (_407)
    {
        _427 = ((all(bool4(_409.x == 17.0f.xxxx.x, _409.y == 17.0f.xxxx.y, _409.z == 17.0f.xxxx.z, _409.w == 17.0f.xxxx.w)) && all(bool4(_410.x == 17.0f.xxxx.x, _410.y == 17.0f.xxxx.y, _410.z == 17.0f.xxxx.z, _410.w == 17.0f.xxxx.w))) && all(bool4(_411.x == 17.0f.xxxx.x, _411.y == 17.0f.xxxx.y, _411.z == 17.0f.xxxx.z, _411.w == 17.0f.xxxx.w))) && all(bool4(_412.x == 17.0f.xxxx.x, _412.y == 17.0f.xxxx.y, _412.z == 17.0f.xxxx.z, _412.w == 17.0f.xxxx.w));
    }
    else
    {
        _427 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _427;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_5_m = float2x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f));
    float2 _429 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _430 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    _RESERVED_IDENTIFIER_FIXUP_5_m = float2x2(_429, _430);
    bool _439 = false;
    if (_427)
    {
        _439 = all(bool2(_429.x == float2(9.0f, 18.0f).x, _429.y == float2(9.0f, 18.0f).y)) && all(bool2(_430.x == float2(27.0f, 36.0f).x, _430.y == float2(27.0f, 36.0f).y));
    }
    else
    {
        _439 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _439;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_6_m = float2x2(float2(2.0f, 4.0f), float2(6.0f, 8.0f));
    float2 _441 = float2(2.0f, 4.0f) / 2.0f.xx;
    float2 _442 = float2(6.0f, 8.0f) / float2(2.0f, 4.0f);
    _RESERVED_IDENTIFIER_FIXUP_6_m = float2x2(_441, _442);
    bool _451 = false;
    if (_439)
    {
        _451 = all(bool2(_441.x == float2(1.0f, 2.0f).x, _441.y == float2(1.0f, 2.0f).y)) && all(bool2(_442.x == float2(3.0f, 2.0f).x, _442.y == float2(3.0f, 2.0f).y));
    }
    else
    {
        _451 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _451;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_7_m = float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f));
    float2x2 _453 = mul(float2x2(float2(3.0f, 5.0f), float2(3.0f, 2.0f)), float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f)));
    _RESERVED_IDENTIFIER_FIXUP_7_m = _453;
    bool _463 = false;
    if (_451)
    {
        float2 _456 = _453[0];
        float2 _459 = _453[1];
        _463 = all(bool2(_456.x == float2(38.0f, 26.0f).x, _456.y == float2(38.0f, 26.0f).y)) && all(bool2(_459.x == float2(17.0f, 14.0f).x, _459.y == float2(17.0f, 14.0f).y));
    }
    else
    {
        _463 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _463;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_8_m = float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f));
    float3x3 _465 = mul(float3x3(float3(3.0f, 3.0f, 4.0f), float3(2.0f, 3.0f, 4.0f), float3(4.0f, 9.0f, 2.0f)), float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f)));
    _RESERVED_IDENTIFIER_FIXUP_8_m = _465;
    bool _479 = false;
    if (_463)
    {
        float3 _468 = _465[0];
        float3 _471 = _465[1];
        float3 _475 = _465[2];
        _479 = (all(bool3(_468.x == float3(130.0f, 51.0f, 35.0f).x, _468.y == float3(130.0f, 51.0f, 35.0f).y, _468.z == float3(130.0f, 51.0f, 35.0f).z)) && all(bool3(_471.x == float3(120.0f, 47.0f, 33.0f).x, _471.y == float3(120.0f, 47.0f, 33.0f).y, _471.z == float3(120.0f, 47.0f, 33.0f).z))) && all(bool3(_475.x == float3(240.0f, 73.0f, 45.0f).x, _475.y == float3(240.0f, 73.0f, 45.0f).y, _475.z == float3(240.0f, 73.0f, 45.0f).z));
    }
    else
    {
        _479 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _479;
    bool _483 = false;
    if (_479)
    {
        _483 = test_matrix_op_matrix_half_b();
    }
    else
    {
        _483 = false;
    }
    float4 _484 = 0.0f.xxxx;
    if (_483)
    {
        _484 = _8_colorGreen;
    }
    else
    {
        _484 = _8_colorRed;
    }
    return _484;
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
