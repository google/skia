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
    float3x2 splat_4 = float3x2(4.0f.xx, 4.0f.xx, 4.0f.xx);
    float3x2 m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _38 = float2(2.0f, 0.0f) + 4.0f.xx;
    float2 _39 = float2(0.0f, 2.0f) + 4.0f.xx;
    float2 _40 = 0.0f.xx + 4.0f.xx;
    m = float3x2(_38, _39, _40);
    bool _58 = false;
    if (true)
    {
        _58 = (all(bool2(_38.x == float2(6.0f, 4.0f).x, _38.y == float2(6.0f, 4.0f).y)) && all(bool2(_39.x == float2(4.0f, 6.0f).x, _39.y == float2(4.0f, 6.0f).y))) && all(bool2(_40.x == 4.0f.xx.x, _40.y == 4.0f.xx.y));
    }
    else
    {
        _58 = false;
    }
    ok = _58;
    m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _59 = float2(2.0f, 0.0f) - 4.0f.xx;
    float2 _60 = float2(0.0f, 2.0f) - 4.0f.xx;
    float2 _61 = 0.0f.xx - 4.0f.xx;
    m = float3x2(_59, _60, _61);
    bool _79 = false;
    if (_58)
    {
        _79 = (all(bool2(_59.x == float2(-2.0f, -4.0f).x, _59.y == float2(-2.0f, -4.0f).y)) && all(bool2(_60.x == float2(-4.0f, -2.0f).x, _60.y == float2(-4.0f, -2.0f).y))) && all(bool2(_61.x == (-4.0f).xx.x, _61.y == (-4.0f).xx.y));
    }
    else
    {
        _79 = false;
    }
    ok = _79;
    m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _80 = float2(2.0f, 0.0f) / 4.0f.xx;
    float2 _81 = float2(0.0f, 2.0f) / 4.0f.xx;
    float2 _82 = 0.0f.xx / 4.0f.xx;
    m = float3x2(_80, _81, _82);
    bool _98 = false;
    if (_79)
    {
        _98 = (all(bool2(_80.x == float2(0.5f, 0.0f).x, _80.y == float2(0.5f, 0.0f).y)) && all(bool2(_81.x == float2(0.0f, 0.5f).x, _81.y == float2(0.0f, 0.5f).y))) && all(bool2(_82.x == 0.0f.xx.x, _82.y == 0.0f.xx.y));
    }
    else
    {
        _98 = false;
    }
    ok = _98;
    float2x3 splat_4_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float2x3 m_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _109 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _110 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    m_1 = float2x3(_109, _110);
    bool _123 = false;
    if (_98)
    {
        _123 = all(bool3(_109.x == float3(6.0f, 4.0f, 4.0f).x, _109.y == float3(6.0f, 4.0f, 4.0f).y, _109.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_110.x == float3(4.0f, 6.0f, 4.0f).x, _110.y == float3(4.0f, 6.0f, 4.0f).y, _110.z == float3(4.0f, 6.0f, 4.0f).z));
    }
    else
    {
        _123 = false;
    }
    ok = _123;
    m_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _124 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _125 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    m_1 = float2x3(_124, _125);
    bool _137 = false;
    if (_123)
    {
        _137 = all(bool3(_124.x == float3(2.0f, 4.0f, 4.0f).x, _124.y == float3(2.0f, 4.0f, 4.0f).y, _124.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_125.x == float3(4.0f, 2.0f, 4.0f).x, _125.y == float3(4.0f, 2.0f, 4.0f).y, _125.z == float3(4.0f, 2.0f, 4.0f).z));
    }
    else
    {
        _137 = false;
    }
    ok = _137;
    m_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _140 = 4.0f.xxx / 2.0f.xxx;
    float3 _141 = 4.0f.xxx / 2.0f.xxx;
    m_1 = float2x3(_140, _141);
    bool _150 = false;
    if (_137)
    {
        _150 = all(bool3(_140.x == 2.0f.xxx.x, _140.y == 2.0f.xxx.y, _140.z == 2.0f.xxx.z)) && all(bool3(_141.x == 2.0f.xxx.x, _141.y == 2.0f.xxx.y, _141.z == 2.0f.xxx.z));
    }
    else
    {
        _150 = false;
    }
    ok = _150;
    float4x3 m_2 = float4x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), float3(10.0f, 11.0f, 12.0f));
    float3 _177 = float3(1.0f, 2.0f, 3.0f) + float3(16.0f, 15.0f, 14.0f);
    float3 _178 = float3(4.0f, 5.0f, 6.0f) + float3(13.0f, 12.0f, 11.0f);
    float3 _179 = float3(7.0f, 8.0f, 9.0f) + float3(10.0f, 9.0f, 8.0f);
    float3 _180 = float3(10.0f, 11.0f, 12.0f) + float3(7.0f, 6.0f, 5.0f);
    m_2 = float4x3(_177, _178, _179, _180);
    bool _198 = false;
    if (_150)
    {
        _198 = ((all(bool3(_177.x == 17.0f.xxx.x, _177.y == 17.0f.xxx.y, _177.z == 17.0f.xxx.z)) && all(bool3(_178.x == 17.0f.xxx.x, _178.y == 17.0f.xxx.y, _178.z == 17.0f.xxx.z))) && all(bool3(_179.x == 17.0f.xxx.x, _179.y == 17.0f.xxx.y, _179.z == 17.0f.xxx.z))) && all(bool3(_180.x == 17.0f.xxx.x, _180.y == 17.0f.xxx.y, _180.z == 17.0f.xxx.z));
    }
    else
    {
        _198 = false;
    }
    ok = _198;
    float4x2 m_3 = float4x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f), float2(50.0f, 60.0f), float2(70.0f, 80.0f));
    float2 _219 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _220 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    float2 _221 = float2(50.0f, 60.0f) - float2(5.0f, 6.0f);
    float2 _222 = float2(70.0f, 80.0f) - float2(7.0f, 8.0f);
    m_3 = float4x2(_219, _220, _221, _222);
    bool _249 = false;
    if (_198)
    {
        _249 = ((all(bool2(_219.x == float2(9.0f, 18.0f).x, _219.y == float2(9.0f, 18.0f).y)) && all(bool2(_220.x == float2(27.0f, 36.0f).x, _220.y == float2(27.0f, 36.0f).y))) && all(bool2(_221.x == float2(45.0f, 54.0f).x, _221.y == float2(45.0f, 54.0f).y))) && all(bool2(_222.x == float2(63.0f, 72.0f).x, _222.y == float2(63.0f, 72.0f).y));
    }
    else
    {
        _249 = false;
    }
    ok = _249;
    float2x4 m_4 = float2x4(float4(10.0f, 20.0f, 30.0f, 40.0f), float4(10.0f, 20.0f, 30.0f, 40.0f));
    float4 _258 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 10.0f.xxxx;
    float4 _259 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 5.0f.xxxx;
    m_4 = float2x4(_258, _259);
    bool _272 = false;
    if (_249)
    {
        _272 = all(bool4(_258.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _258.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _258.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _258.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_259.x == float4(2.0f, 4.0f, 6.0f, 8.0f).x, _259.y == float4(2.0f, 4.0f, 6.0f, 8.0f).y, _259.z == float4(2.0f, 4.0f, 6.0f, 8.0f).z, _259.w == float4(2.0f, 4.0f, 6.0f, 8.0f).w));
    }
    else
    {
        _272 = false;
    }
    ok = _272;
    float2x3 m_5 = float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f));
    float2x3 _281 = mul(float2x2(float2(1.0f, 4.0f), float2(2.0f, 5.0f)), float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f)));
    m_5 = _281;
    bool _299 = false;
    if (_272)
    {
        float3 _292 = _281[0];
        float3 _295 = _281[1];
        _299 = all(bool3(_292.x == float3(39.0f, 49.0f, 59.0f).x, _292.y == float3(39.0f, 49.0f, 59.0f).y, _292.z == float3(39.0f, 49.0f, 59.0f).z)) && all(bool3(_295.x == float3(54.0f, 68.0f, 82.0f).x, _295.y == float3(54.0f, 68.0f, 82.0f).y, _295.z == float3(54.0f, 68.0f, 82.0f).z));
    }
    else
    {
        _299 = false;
    }
    ok = _299;
    return _299;
}

float4 main(float2 _301)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float3x2 _RESERVED_IDENTIFIER_FIXUP_1_splat_4 = float3x2(4.0f.xx, 4.0f.xx, 4.0f.xx);
    float3x2 _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _306 = float2(2.0f, 0.0f) + 4.0f.xx;
    float2 _307 = float2(0.0f, 2.0f) + 4.0f.xx;
    float2 _308 = 0.0f.xx + 4.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(_306, _307, _308);
    bool _320 = false;
    if (true)
    {
        _320 = (all(bool2(_306.x == float2(6.0f, 4.0f).x, _306.y == float2(6.0f, 4.0f).y)) && all(bool2(_307.x == float2(4.0f, 6.0f).x, _307.y == float2(4.0f, 6.0f).y))) && all(bool2(_308.x == 4.0f.xx.x, _308.y == 4.0f.xx.y));
    }
    else
    {
        _320 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _320;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _321 = float2(2.0f, 0.0f) - 4.0f.xx;
    float2 _322 = float2(0.0f, 2.0f) - 4.0f.xx;
    float2 _323 = 0.0f.xx - 4.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(_321, _322, _323);
    bool _335 = false;
    if (_320)
    {
        _335 = (all(bool2(_321.x == float2(-2.0f, -4.0f).x, _321.y == float2(-2.0f, -4.0f).y)) && all(bool2(_322.x == float2(-4.0f, -2.0f).x, _322.y == float2(-4.0f, -2.0f).y))) && all(bool2(_323.x == (-4.0f).xx.x, _323.y == (-4.0f).xx.y));
    }
    else
    {
        _335 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _335;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _336 = float2(2.0f, 0.0f) / 4.0f.xx;
    float2 _337 = float2(0.0f, 2.0f) / 4.0f.xx;
    float2 _338 = 0.0f.xx / 4.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(_336, _337, _338);
    bool _350 = false;
    if (_335)
    {
        _350 = (all(bool2(_336.x == float2(0.5f, 0.0f).x, _336.y == float2(0.5f, 0.0f).y)) && all(bool2(_337.x == float2(0.0f, 0.5f).x, _337.y == float2(0.0f, 0.5f).y))) && all(bool2(_338.x == 0.0f.xx.x, _338.y == 0.0f.xx.y));
    }
    else
    {
        _350 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _350;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_3_splat_4 = float2x3(4.0f.xxx, 4.0f.xxx);
    float2x3 _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _353 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _354 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(_353, _354);
    bool _363 = false;
    if (_350)
    {
        _363 = all(bool3(_353.x == float3(6.0f, 4.0f, 4.0f).x, _353.y == float3(6.0f, 4.0f, 4.0f).y, _353.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_354.x == float3(4.0f, 6.0f, 4.0f).x, _354.y == float3(4.0f, 6.0f, 4.0f).y, _354.z == float3(4.0f, 6.0f, 4.0f).z));
    }
    else
    {
        _363 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _363;
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _364 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _365 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(_364, _365);
    bool _374 = false;
    if (_363)
    {
        _374 = all(bool3(_364.x == float3(2.0f, 4.0f, 4.0f).x, _364.y == float3(2.0f, 4.0f, 4.0f).y, _364.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_365.x == float3(4.0f, 2.0f, 4.0f).x, _365.y == float3(4.0f, 2.0f, 4.0f).y, _365.z == float3(4.0f, 2.0f, 4.0f).z));
    }
    else
    {
        _374 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _374;
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _375 = 4.0f.xxx / 2.0f.xxx;
    float3 _376 = 4.0f.xxx / 2.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(_375, _376);
    bool _385 = false;
    if (_374)
    {
        _385 = all(bool3(_375.x == 2.0f.xxx.x, _375.y == 2.0f.xxx.y, _375.z == 2.0f.xxx.z)) && all(bool3(_376.x == 2.0f.xxx.x, _376.y == 2.0f.xxx.y, _376.z == 2.0f.xxx.z));
    }
    else
    {
        _385 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _385;
    float4x3 _RESERVED_IDENTIFIER_FIXUP_5_m = float4x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), float3(10.0f, 11.0f, 12.0f));
    float3 _387 = float3(1.0f, 2.0f, 3.0f) + float3(16.0f, 15.0f, 14.0f);
    float3 _388 = float3(4.0f, 5.0f, 6.0f) + float3(13.0f, 12.0f, 11.0f);
    float3 _389 = float3(7.0f, 8.0f, 9.0f) + float3(10.0f, 9.0f, 8.0f);
    float3 _390 = float3(10.0f, 11.0f, 12.0f) + float3(7.0f, 6.0f, 5.0f);
    _RESERVED_IDENTIFIER_FIXUP_5_m = float4x3(_387, _388, _389, _390);
    bool _405 = false;
    if (_385)
    {
        _405 = ((all(bool3(_387.x == 17.0f.xxx.x, _387.y == 17.0f.xxx.y, _387.z == 17.0f.xxx.z)) && all(bool3(_388.x == 17.0f.xxx.x, _388.y == 17.0f.xxx.y, _388.z == 17.0f.xxx.z))) && all(bool3(_389.x == 17.0f.xxx.x, _389.y == 17.0f.xxx.y, _389.z == 17.0f.xxx.z))) && all(bool3(_390.x == 17.0f.xxx.x, _390.y == 17.0f.xxx.y, _390.z == 17.0f.xxx.z));
    }
    else
    {
        _405 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _405;
    float4x2 _RESERVED_IDENTIFIER_FIXUP_6_m = float4x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f), float2(50.0f, 60.0f), float2(70.0f, 80.0f));
    float2 _407 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _408 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    float2 _409 = float2(50.0f, 60.0f) - float2(5.0f, 6.0f);
    float2 _410 = float2(70.0f, 80.0f) - float2(7.0f, 8.0f);
    _RESERVED_IDENTIFIER_FIXUP_6_m = float4x2(_407, _408, _409, _410);
    bool _425 = false;
    if (_405)
    {
        _425 = ((all(bool2(_407.x == float2(9.0f, 18.0f).x, _407.y == float2(9.0f, 18.0f).y)) && all(bool2(_408.x == float2(27.0f, 36.0f).x, _408.y == float2(27.0f, 36.0f).y))) && all(bool2(_409.x == float2(45.0f, 54.0f).x, _409.y == float2(45.0f, 54.0f).y))) && all(bool2(_410.x == float2(63.0f, 72.0f).x, _410.y == float2(63.0f, 72.0f).y));
    }
    else
    {
        _425 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _425;
    float2x4 _RESERVED_IDENTIFIER_FIXUP_7_m = float2x4(float4(10.0f, 20.0f, 30.0f, 40.0f), float4(10.0f, 20.0f, 30.0f, 40.0f));
    float4 _427 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 10.0f.xxxx;
    float4 _428 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 5.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_7_m = float2x4(_427, _428);
    bool _437 = false;
    if (_425)
    {
        _437 = all(bool4(_427.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _427.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _427.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _427.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_428.x == float4(2.0f, 4.0f, 6.0f, 8.0f).x, _428.y == float4(2.0f, 4.0f, 6.0f, 8.0f).y, _428.z == float4(2.0f, 4.0f, 6.0f, 8.0f).z, _428.w == float4(2.0f, 4.0f, 6.0f, 8.0f).w));
    }
    else
    {
        _437 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _437;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_8_m = float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f));
    float2x3 _439 = mul(float2x2(float2(1.0f, 4.0f), float2(2.0f, 5.0f)), float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f)));
    _RESERVED_IDENTIFIER_FIXUP_8_m = _439;
    bool _449 = false;
    if (_437)
    {
        float3 _442 = _439[0];
        float3 _445 = _439[1];
        _449 = all(bool3(_442.x == float3(39.0f, 49.0f, 59.0f).x, _442.y == float3(39.0f, 49.0f, 59.0f).y, _442.z == float3(39.0f, 49.0f, 59.0f).z)) && all(bool3(_445.x == float3(54.0f, 68.0f, 82.0f).x, _445.y == float3(54.0f, 68.0f, 82.0f).y, _445.z == float3(54.0f, 68.0f, 82.0f).z));
    }
    else
    {
        _449 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _449;
    bool _453 = false;
    if (_449)
    {
        _453 = test_matrix_op_matrix_half_b();
    }
    else
    {
        _453 = false;
    }
    float4 _454 = 0.0f.xxxx;
    if (_453)
    {
        _454 = _8_colorGreen;
    }
    else
    {
        _454 = _8_colorRed;
    }
    return _454;
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
