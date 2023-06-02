cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorRed : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
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
    float3 _45 = float3(2.0f, 0.0f, 0.0f) + 4.0f.xxx;
    float3 _46 = float3(0.0f, 2.0f, 0.0f) + 4.0f.xxx;
    float3 _47 = float3(0.0f, 0.0f, 2.0f) + 4.0f.xxx;
    m = float3x3(_45, _46, _47);
    bool _66 = false;
    if (true)
    {
        _66 = (all(bool3(_45.x == float3(6.0f, 4.0f, 4.0f).x, _45.y == float3(6.0f, 4.0f, 4.0f).y, _45.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_46.x == float3(4.0f, 6.0f, 4.0f).x, _46.y == float3(4.0f, 6.0f, 4.0f).y, _46.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_47.x == float3(4.0f, 4.0f, 6.0f).x, _47.y == float3(4.0f, 4.0f, 6.0f).y, _47.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _66 = false;
    }
    ok = _66;
    m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _67 = float3(2.0f, 0.0f, 0.0f) - 4.0f.xxx;
    float3 _68 = float3(0.0f, 2.0f, 0.0f) - 4.0f.xxx;
    float3 _69 = float3(0.0f, 0.0f, 2.0f) - 4.0f.xxx;
    m = float3x3(_67, _68, _69);
    bool _87 = false;
    if (_66)
    {
        _87 = (all(bool3(_67.x == float3(-2.0f, -4.0f, -4.0f).x, _67.y == float3(-2.0f, -4.0f, -4.0f).y, _67.z == float3(-2.0f, -4.0f, -4.0f).z)) && all(bool3(_68.x == float3(-4.0f, -2.0f, -4.0f).x, _68.y == float3(-4.0f, -2.0f, -4.0f).y, _68.z == float3(-4.0f, -2.0f, -4.0f).z))) && all(bool3(_69.x == float3(-4.0f, -4.0f, -2.0f).x, _69.y == float3(-4.0f, -4.0f, -2.0f).y, _69.z == float3(-4.0f, -4.0f, -2.0f).z));
    }
    else
    {
        _87 = false;
    }
    ok = _87;
    m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _88 = float3(2.0f, 0.0f, 0.0f) / 4.0f.xxx;
    float3 _89 = float3(0.0f, 2.0f, 0.0f) / 4.0f.xxx;
    float3 _90 = float3(0.0f, 0.0f, 2.0f) / 4.0f.xxx;
    m = float3x3(_88, _89, _90);
    bool _107 = false;
    if (_87)
    {
        _107 = (all(bool3(_88.x == float3(0.5f, 0.0f, 0.0f).x, _88.y == float3(0.5f, 0.0f, 0.0f).y, _88.z == float3(0.5f, 0.0f, 0.0f).z)) && all(bool3(_89.x == float3(0.0f, 0.5f, 0.0f).x, _89.y == float3(0.0f, 0.5f, 0.0f).y, _89.z == float3(0.0f, 0.5f, 0.0f).z))) && all(bool3(_90.x == float3(0.0f, 0.0f, 0.5f).x, _90.y == float3(0.0f, 0.0f, 0.5f).y, _90.z == float3(0.0f, 0.0f, 0.5f).z));
    }
    else
    {
        _107 = false;
    }
    ok = _107;
    m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _108 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _109 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    float3 _110 = 4.0f.xxx + float3(0.0f, 0.0f, 2.0f);
    m = float3x3(_108, _109, _110);
    bool _122 = false;
    if (_107)
    {
        _122 = (all(bool3(_108.x == float3(6.0f, 4.0f, 4.0f).x, _108.y == float3(6.0f, 4.0f, 4.0f).y, _108.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_109.x == float3(4.0f, 6.0f, 4.0f).x, _109.y == float3(4.0f, 6.0f, 4.0f).y, _109.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_110.x == float3(4.0f, 4.0f, 6.0f).x, _110.y == float3(4.0f, 4.0f, 6.0f).y, _110.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _122 = false;
    }
    ok = _122;
    m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _123 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _124 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    float3 _125 = 4.0f.xxx - float3(0.0f, 0.0f, 2.0f);
    m = float3x3(_123, _124, _125);
    bool _141 = false;
    if (_122)
    {
        _141 = (all(bool3(_123.x == float3(2.0f, 4.0f, 4.0f).x, _123.y == float3(2.0f, 4.0f, 4.0f).y, _123.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_124.x == float3(4.0f, 2.0f, 4.0f).x, _124.y == float3(4.0f, 2.0f, 4.0f).y, _124.z == float3(4.0f, 2.0f, 4.0f).z))) && all(bool3(_125.x == float3(4.0f, 4.0f, 2.0f).x, _125.y == float3(4.0f, 4.0f, 2.0f).y, _125.z == float3(4.0f, 4.0f, 2.0f).z));
    }
    else
    {
        _141 = false;
    }
    ok = _141;
    m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _142 = 4.0f.xxx / 2.0f.xxx;
    float3 _143 = 4.0f.xxx / 2.0f.xxx;
    float3 _144 = 4.0f.xxx / 2.0f.xxx;
    m = float3x3(_142, _143, _144);
    bool _156 = false;
    if (_141)
    {
        _156 = (all(bool3(_142.x == 2.0f.xxx.x, _142.y == 2.0f.xxx.y, _142.z == 2.0f.xxx.z)) && all(bool3(_143.x == 2.0f.xxx.x, _143.y == 2.0f.xxx.y, _143.z == 2.0f.xxx.z))) && all(bool3(_144.x == 2.0f.xxx.x, _144.y == 2.0f.xxx.y, _144.z == 2.0f.xxx.z));
    }
    else
    {
        _156 = false;
    }
    ok = _156;
    float4x4 m_1 = float4x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f), float4(13.0f, 14.0f, 15.0f, 16.0f));
    float4 _183 = float4(1.0f, 2.0f, 3.0f, 4.0f) + float4(16.0f, 15.0f, 14.0f, 13.0f);
    float4 _184 = float4(5.0f, 6.0f, 7.0f, 8.0f) + float4(12.0f, 11.0f, 10.0f, 9.0f);
    float4 _185 = float4(9.0f, 10.0f, 11.0f, 12.0f) + float4(8.0f, 7.0f, 6.0f, 5.0f);
    float4 _186 = float4(13.0f, 14.0f, 15.0f, 16.0f) + float4(4.0f, 3.0f, 2.0f, 1.0f);
    m_1 = float4x4(_183, _184, _185, _186);
    bool _205 = false;
    if (_156)
    {
        _205 = ((all(bool4(_183.x == 17.0f.xxxx.x, _183.y == 17.0f.xxxx.y, _183.z == 17.0f.xxxx.z, _183.w == 17.0f.xxxx.w)) && all(bool4(_184.x == 17.0f.xxxx.x, _184.y == 17.0f.xxxx.y, _184.z == 17.0f.xxxx.z, _184.w == 17.0f.xxxx.w))) && all(bool4(_185.x == 17.0f.xxxx.x, _185.y == 17.0f.xxxx.y, _185.z == 17.0f.xxxx.z, _185.w == 17.0f.xxxx.w))) && all(bool4(_186.x == 17.0f.xxxx.x, _186.y == 17.0f.xxxx.y, _186.z == 17.0f.xxxx.z, _186.w == 17.0f.xxxx.w));
    }
    else
    {
        _205 = false;
    }
    ok = _205;
    float2x2 m_2 = float2x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f));
    float2 _218 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _219 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    m_2 = float2x2(_218, _219);
    bool _235 = false;
    if (_205)
    {
        _235 = all(bool2(_218.x == float2(9.0f, 18.0f).x, _218.y == float2(9.0f, 18.0f).y)) && all(bool2(_219.x == float2(27.0f, 36.0f).x, _219.y == float2(27.0f, 36.0f).y));
    }
    else
    {
        _235 = false;
    }
    ok = _235;
    float2x2 m_3 = float2x2(float2(2.0f, 4.0f), float2(6.0f, 8.0f));
    float2 _242 = float2(2.0f, 4.0f) / 2.0f.xx;
    float2 _243 = float2(6.0f, 8.0f) / float2(2.0f, 4.0f);
    m_3 = float2x2(_242, _243);
    bool _254 = false;
    if (_235)
    {
        _254 = all(bool2(_242.x == float2(1.0f, 2.0f).x, _242.y == float2(1.0f, 2.0f).y)) && all(bool2(_243.x == float2(3.0f, 2.0f).x, _243.y == float2(3.0f, 2.0f).y));
    }
    else
    {
        _254 = false;
    }
    ok = _254;
    float2x2 m_4 = float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f));
    float2x2 _260 = mul(float2x2(float2(3.0f, 5.0f), float2(3.0f, 2.0f)), float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f)));
    m_4 = _260;
    bool _275 = false;
    if (_254)
    {
        float2 _268 = _260[0];
        float2 _271 = _260[1];
        _275 = all(bool2(_268.x == float2(38.0f, 26.0f).x, _268.y == float2(38.0f, 26.0f).y)) && all(bool2(_271.x == float2(17.0f, 14.0f).x, _271.y == float2(17.0f, 14.0f).y));
    }
    else
    {
        _275 = false;
    }
    ok = _275;
    float3x3 m_5 = float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f));
    float3x3 _285 = mul(float3x3(float3(3.0f, 3.0f, 4.0f), float3(2.0f, 3.0f, 4.0f), float3(4.0f, 9.0f, 2.0f)), float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f)));
    m_5 = _285;
    bool _312 = false;
    if (_275)
    {
        float3 _301 = _285[0];
        float3 _304 = _285[1];
        float3 _308 = _285[2];
        _312 = (all(bool3(_301.x == float3(130.0f, 51.0f, 35.0f).x, _301.y == float3(130.0f, 51.0f, 35.0f).y, _301.z == float3(130.0f, 51.0f, 35.0f).z)) && all(bool3(_304.x == float3(120.0f, 47.0f, 33.0f).x, _304.y == float3(120.0f, 47.0f, 33.0f).y, _304.z == float3(120.0f, 47.0f, 33.0f).z))) && all(bool3(_308.x == float3(240.0f, 73.0f, 45.0f).x, _308.y == float3(240.0f, 73.0f, 45.0f).y, _308.z == float3(240.0f, 73.0f, 45.0f).z));
    }
    else
    {
        _312 = false;
    }
    ok = _312;
    return _312;
}

float4 main(float2 _314)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_1_splat_4 = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3x3 _RESERVED_IDENTIFIER_FIXUP_2_splat_2 = float3x3(2.0f.xxx, 2.0f.xxx, 2.0f.xxx);
    float3x3 _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _320 = float3(2.0f, 0.0f, 0.0f) + 4.0f.xxx;
    float3 _321 = float3(0.0f, 2.0f, 0.0f) + 4.0f.xxx;
    float3 _322 = float3(0.0f, 0.0f, 2.0f) + 4.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_320, _321, _322);
    bool _334 = false;
    if (true)
    {
        _334 = (all(bool3(_320.x == float3(6.0f, 4.0f, 4.0f).x, _320.y == float3(6.0f, 4.0f, 4.0f).y, _320.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_321.x == float3(4.0f, 6.0f, 4.0f).x, _321.y == float3(4.0f, 6.0f, 4.0f).y, _321.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_322.x == float3(4.0f, 4.0f, 6.0f).x, _322.y == float3(4.0f, 4.0f, 6.0f).y, _322.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _334 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _334;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _335 = float3(2.0f, 0.0f, 0.0f) - 4.0f.xxx;
    float3 _336 = float3(0.0f, 2.0f, 0.0f) - 4.0f.xxx;
    float3 _337 = float3(0.0f, 0.0f, 2.0f) - 4.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_335, _336, _337);
    bool _349 = false;
    if (_334)
    {
        _349 = (all(bool3(_335.x == float3(-2.0f, -4.0f, -4.0f).x, _335.y == float3(-2.0f, -4.0f, -4.0f).y, _335.z == float3(-2.0f, -4.0f, -4.0f).z)) && all(bool3(_336.x == float3(-4.0f, -2.0f, -4.0f).x, _336.y == float3(-4.0f, -2.0f, -4.0f).y, _336.z == float3(-4.0f, -2.0f, -4.0f).z))) && all(bool3(_337.x == float3(-4.0f, -4.0f, -2.0f).x, _337.y == float3(-4.0f, -4.0f, -2.0f).y, _337.z == float3(-4.0f, -4.0f, -2.0f).z));
    }
    else
    {
        _349 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _349;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _350 = float3(2.0f, 0.0f, 0.0f) / 4.0f.xxx;
    float3 _351 = float3(0.0f, 2.0f, 0.0f) / 4.0f.xxx;
    float3 _352 = float3(0.0f, 0.0f, 2.0f) / 4.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_350, _351, _352);
    bool _364 = false;
    if (_349)
    {
        _364 = (all(bool3(_350.x == float3(0.5f, 0.0f, 0.0f).x, _350.y == float3(0.5f, 0.0f, 0.0f).y, _350.z == float3(0.5f, 0.0f, 0.0f).z)) && all(bool3(_351.x == float3(0.0f, 0.5f, 0.0f).x, _351.y == float3(0.0f, 0.5f, 0.0f).y, _351.z == float3(0.0f, 0.5f, 0.0f).z))) && all(bool3(_352.x == float3(0.0f, 0.0f, 0.5f).x, _352.y == float3(0.0f, 0.0f, 0.5f).y, _352.z == float3(0.0f, 0.0f, 0.5f).z));
    }
    else
    {
        _364 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _364;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _365 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _366 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    float3 _367 = 4.0f.xxx + float3(0.0f, 0.0f, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_365, _366, _367);
    bool _379 = false;
    if (_364)
    {
        _379 = (all(bool3(_365.x == float3(6.0f, 4.0f, 4.0f).x, _365.y == float3(6.0f, 4.0f, 4.0f).y, _365.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_366.x == float3(4.0f, 6.0f, 4.0f).x, _366.y == float3(4.0f, 6.0f, 4.0f).y, _366.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_367.x == float3(4.0f, 4.0f, 6.0f).x, _367.y == float3(4.0f, 4.0f, 6.0f).y, _367.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _379 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _379;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _380 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _381 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    float3 _382 = 4.0f.xxx - float3(0.0f, 0.0f, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_380, _381, _382);
    bool _394 = false;
    if (_379)
    {
        _394 = (all(bool3(_380.x == float3(2.0f, 4.0f, 4.0f).x, _380.y == float3(2.0f, 4.0f, 4.0f).y, _380.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_381.x == float3(4.0f, 2.0f, 4.0f).x, _381.y == float3(4.0f, 2.0f, 4.0f).y, _381.z == float3(4.0f, 2.0f, 4.0f).z))) && all(bool3(_382.x == float3(4.0f, 4.0f, 2.0f).x, _382.y == float3(4.0f, 4.0f, 2.0f).y, _382.z == float3(4.0f, 4.0f, 2.0f).z));
    }
    else
    {
        _394 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _394;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _395 = 4.0f.xxx / 2.0f.xxx;
    float3 _396 = 4.0f.xxx / 2.0f.xxx;
    float3 _397 = 4.0f.xxx / 2.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_395, _396, _397);
    bool _409 = false;
    if (_394)
    {
        _409 = (all(bool3(_395.x == 2.0f.xxx.x, _395.y == 2.0f.xxx.y, _395.z == 2.0f.xxx.z)) && all(bool3(_396.x == 2.0f.xxx.x, _396.y == 2.0f.xxx.y, _396.z == 2.0f.xxx.z))) && all(bool3(_397.x == 2.0f.xxx.x, _397.y == 2.0f.xxx.y, _397.z == 2.0f.xxx.z));
    }
    else
    {
        _409 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _409;
    float4x4 _RESERVED_IDENTIFIER_FIXUP_4_m = float4x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f), float4(13.0f, 14.0f, 15.0f, 16.0f));
    float4 _411 = float4(1.0f, 2.0f, 3.0f, 4.0f) + float4(16.0f, 15.0f, 14.0f, 13.0f);
    float4 _412 = float4(5.0f, 6.0f, 7.0f, 8.0f) + float4(12.0f, 11.0f, 10.0f, 9.0f);
    float4 _413 = float4(9.0f, 10.0f, 11.0f, 12.0f) + float4(8.0f, 7.0f, 6.0f, 5.0f);
    float4 _414 = float4(13.0f, 14.0f, 15.0f, 16.0f) + float4(4.0f, 3.0f, 2.0f, 1.0f);
    _RESERVED_IDENTIFIER_FIXUP_4_m = float4x4(_411, _412, _413, _414);
    bool _429 = false;
    if (_409)
    {
        _429 = ((all(bool4(_411.x == 17.0f.xxxx.x, _411.y == 17.0f.xxxx.y, _411.z == 17.0f.xxxx.z, _411.w == 17.0f.xxxx.w)) && all(bool4(_412.x == 17.0f.xxxx.x, _412.y == 17.0f.xxxx.y, _412.z == 17.0f.xxxx.z, _412.w == 17.0f.xxxx.w))) && all(bool4(_413.x == 17.0f.xxxx.x, _413.y == 17.0f.xxxx.y, _413.z == 17.0f.xxxx.z, _413.w == 17.0f.xxxx.w))) && all(bool4(_414.x == 17.0f.xxxx.x, _414.y == 17.0f.xxxx.y, _414.z == 17.0f.xxxx.z, _414.w == 17.0f.xxxx.w));
    }
    else
    {
        _429 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _429;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_5_m = float2x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f));
    float2 _431 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _432 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    _RESERVED_IDENTIFIER_FIXUP_5_m = float2x2(_431, _432);
    bool _441 = false;
    if (_429)
    {
        _441 = all(bool2(_431.x == float2(9.0f, 18.0f).x, _431.y == float2(9.0f, 18.0f).y)) && all(bool2(_432.x == float2(27.0f, 36.0f).x, _432.y == float2(27.0f, 36.0f).y));
    }
    else
    {
        _441 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _441;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_6_m = float2x2(float2(2.0f, 4.0f), float2(6.0f, 8.0f));
    float2 _443 = float2(2.0f, 4.0f) / 2.0f.xx;
    float2 _444 = float2(6.0f, 8.0f) / float2(2.0f, 4.0f);
    _RESERVED_IDENTIFIER_FIXUP_6_m = float2x2(_443, _444);
    bool _453 = false;
    if (_441)
    {
        _453 = all(bool2(_443.x == float2(1.0f, 2.0f).x, _443.y == float2(1.0f, 2.0f).y)) && all(bool2(_444.x == float2(3.0f, 2.0f).x, _444.y == float2(3.0f, 2.0f).y));
    }
    else
    {
        _453 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _453;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_7_m = float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f));
    float2x2 _455 = mul(float2x2(float2(3.0f, 5.0f), float2(3.0f, 2.0f)), float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f)));
    _RESERVED_IDENTIFIER_FIXUP_7_m = _455;
    bool _465 = false;
    if (_453)
    {
        float2 _458 = _455[0];
        float2 _461 = _455[1];
        _465 = all(bool2(_458.x == float2(38.0f, 26.0f).x, _458.y == float2(38.0f, 26.0f).y)) && all(bool2(_461.x == float2(17.0f, 14.0f).x, _461.y == float2(17.0f, 14.0f).y));
    }
    else
    {
        _465 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _465;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_8_m = float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f));
    float3x3 _467 = mul(float3x3(float3(3.0f, 3.0f, 4.0f), float3(2.0f, 3.0f, 4.0f), float3(4.0f, 9.0f, 2.0f)), float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f)));
    _RESERVED_IDENTIFIER_FIXUP_8_m = _467;
    bool _481 = false;
    if (_465)
    {
        float3 _470 = _467[0];
        float3 _473 = _467[1];
        float3 _477 = _467[2];
        _481 = (all(bool3(_470.x == float3(130.0f, 51.0f, 35.0f).x, _470.y == float3(130.0f, 51.0f, 35.0f).y, _470.z == float3(130.0f, 51.0f, 35.0f).z)) && all(bool3(_473.x == float3(120.0f, 47.0f, 33.0f).x, _473.y == float3(120.0f, 47.0f, 33.0f).y, _473.z == float3(120.0f, 47.0f, 33.0f).z))) && all(bool3(_477.x == float3(240.0f, 73.0f, 45.0f).x, _477.y == float3(240.0f, 73.0f, 45.0f).y, _477.z == float3(240.0f, 73.0f, 45.0f).z));
    }
    else
    {
        _481 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _481;
    bool _485 = false;
    if (_481)
    {
        _485 = test_matrix_op_matrix_half_b();
    }
    else
    {
        _485 = false;
    }
    float4 _486 = 0.0f.xxxx;
    if (_485)
    {
        _486 = _11_colorGreen;
    }
    else
    {
        _486 = _11_colorRed;
    }
    return _486;
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
