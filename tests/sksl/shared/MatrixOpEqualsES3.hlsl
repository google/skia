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
    float3x2 splat_4 = float3x2(4.0f.xx, 4.0f.xx, 4.0f.xx);
    float3x2 m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _40 = float2(2.0f, 0.0f) + 4.0f.xx;
    float2 _41 = float2(0.0f, 2.0f) + 4.0f.xx;
    float2 _42 = 0.0f.xx + 4.0f.xx;
    m = float3x2(_40, _41, _42);
    bool _60 = false;
    if (true)
    {
        _60 = (all(bool2(_40.x == float2(6.0f, 4.0f).x, _40.y == float2(6.0f, 4.0f).y)) && all(bool2(_41.x == float2(4.0f, 6.0f).x, _41.y == float2(4.0f, 6.0f).y))) && all(bool2(_42.x == 4.0f.xx.x, _42.y == 4.0f.xx.y));
    }
    else
    {
        _60 = false;
    }
    ok = _60;
    m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _61 = float2(2.0f, 0.0f) - 4.0f.xx;
    float2 _62 = float2(0.0f, 2.0f) - 4.0f.xx;
    float2 _63 = 0.0f.xx - 4.0f.xx;
    m = float3x2(_61, _62, _63);
    bool _81 = false;
    if (_60)
    {
        _81 = (all(bool2(_61.x == float2(-2.0f, -4.0f).x, _61.y == float2(-2.0f, -4.0f).y)) && all(bool2(_62.x == float2(-4.0f, -2.0f).x, _62.y == float2(-4.0f, -2.0f).y))) && all(bool2(_63.x == (-4.0f).xx.x, _63.y == (-4.0f).xx.y));
    }
    else
    {
        _81 = false;
    }
    ok = _81;
    m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _82 = float2(2.0f, 0.0f) / 4.0f.xx;
    float2 _83 = float2(0.0f, 2.0f) / 4.0f.xx;
    float2 _84 = 0.0f.xx / 4.0f.xx;
    m = float3x2(_82, _83, _84);
    bool _100 = false;
    if (_81)
    {
        _100 = (all(bool2(_82.x == float2(0.5f, 0.0f).x, _82.y == float2(0.5f, 0.0f).y)) && all(bool2(_83.x == float2(0.0f, 0.5f).x, _83.y == float2(0.0f, 0.5f).y))) && all(bool2(_84.x == 0.0f.xx.x, _84.y == 0.0f.xx.y));
    }
    else
    {
        _100 = false;
    }
    ok = _100;
    float2x3 splat_4_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float2x3 m_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _111 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _112 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    m_1 = float2x3(_111, _112);
    bool _125 = false;
    if (_100)
    {
        _125 = all(bool3(_111.x == float3(6.0f, 4.0f, 4.0f).x, _111.y == float3(6.0f, 4.0f, 4.0f).y, _111.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_112.x == float3(4.0f, 6.0f, 4.0f).x, _112.y == float3(4.0f, 6.0f, 4.0f).y, _112.z == float3(4.0f, 6.0f, 4.0f).z));
    }
    else
    {
        _125 = false;
    }
    ok = _125;
    m_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _126 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _127 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    m_1 = float2x3(_126, _127);
    bool _139 = false;
    if (_125)
    {
        _139 = all(bool3(_126.x == float3(2.0f, 4.0f, 4.0f).x, _126.y == float3(2.0f, 4.0f, 4.0f).y, _126.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_127.x == float3(4.0f, 2.0f, 4.0f).x, _127.y == float3(4.0f, 2.0f, 4.0f).y, _127.z == float3(4.0f, 2.0f, 4.0f).z));
    }
    else
    {
        _139 = false;
    }
    ok = _139;
    m_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _142 = 4.0f.xxx / 2.0f.xxx;
    float3 _143 = 4.0f.xxx / 2.0f.xxx;
    m_1 = float2x3(_142, _143);
    bool _152 = false;
    if (_139)
    {
        _152 = all(bool3(_142.x == 2.0f.xxx.x, _142.y == 2.0f.xxx.y, _142.z == 2.0f.xxx.z)) && all(bool3(_143.x == 2.0f.xxx.x, _143.y == 2.0f.xxx.y, _143.z == 2.0f.xxx.z));
    }
    else
    {
        _152 = false;
    }
    ok = _152;
    float4x3 m_2 = float4x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), float3(10.0f, 11.0f, 12.0f));
    float3 _179 = float3(1.0f, 2.0f, 3.0f) + float3(16.0f, 15.0f, 14.0f);
    float3 _180 = float3(4.0f, 5.0f, 6.0f) + float3(13.0f, 12.0f, 11.0f);
    float3 _181 = float3(7.0f, 8.0f, 9.0f) + float3(10.0f, 9.0f, 8.0f);
    float3 _182 = float3(10.0f, 11.0f, 12.0f) + float3(7.0f, 6.0f, 5.0f);
    m_2 = float4x3(_179, _180, _181, _182);
    bool _200 = false;
    if (_152)
    {
        _200 = ((all(bool3(_179.x == 17.0f.xxx.x, _179.y == 17.0f.xxx.y, _179.z == 17.0f.xxx.z)) && all(bool3(_180.x == 17.0f.xxx.x, _180.y == 17.0f.xxx.y, _180.z == 17.0f.xxx.z))) && all(bool3(_181.x == 17.0f.xxx.x, _181.y == 17.0f.xxx.y, _181.z == 17.0f.xxx.z))) && all(bool3(_182.x == 17.0f.xxx.x, _182.y == 17.0f.xxx.y, _182.z == 17.0f.xxx.z));
    }
    else
    {
        _200 = false;
    }
    ok = _200;
    float4x2 m_3 = float4x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f), float2(50.0f, 60.0f), float2(70.0f, 80.0f));
    float2 _221 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _222 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    float2 _223 = float2(50.0f, 60.0f) - float2(5.0f, 6.0f);
    float2 _224 = float2(70.0f, 80.0f) - float2(7.0f, 8.0f);
    m_3 = float4x2(_221, _222, _223, _224);
    bool _251 = false;
    if (_200)
    {
        _251 = ((all(bool2(_221.x == float2(9.0f, 18.0f).x, _221.y == float2(9.0f, 18.0f).y)) && all(bool2(_222.x == float2(27.0f, 36.0f).x, _222.y == float2(27.0f, 36.0f).y))) && all(bool2(_223.x == float2(45.0f, 54.0f).x, _223.y == float2(45.0f, 54.0f).y))) && all(bool2(_224.x == float2(63.0f, 72.0f).x, _224.y == float2(63.0f, 72.0f).y));
    }
    else
    {
        _251 = false;
    }
    ok = _251;
    float2x4 m_4 = float2x4(float4(10.0f, 20.0f, 30.0f, 40.0f), float4(10.0f, 20.0f, 30.0f, 40.0f));
    float4 _260 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 10.0f.xxxx;
    float4 _261 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 5.0f.xxxx;
    m_4 = float2x4(_260, _261);
    bool _274 = false;
    if (_251)
    {
        _274 = all(bool4(_260.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _260.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _260.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _260.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_261.x == float4(2.0f, 4.0f, 6.0f, 8.0f).x, _261.y == float4(2.0f, 4.0f, 6.0f, 8.0f).y, _261.z == float4(2.0f, 4.0f, 6.0f, 8.0f).z, _261.w == float4(2.0f, 4.0f, 6.0f, 8.0f).w));
    }
    else
    {
        _274 = false;
    }
    ok = _274;
    float2x3 m_5 = float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f));
    float2x3 _283 = mul(float2x2(float2(1.0f, 4.0f), float2(2.0f, 5.0f)), float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f)));
    m_5 = _283;
    bool _301 = false;
    if (_274)
    {
        float3 _294 = _283[0];
        float3 _297 = _283[1];
        _301 = all(bool3(_294.x == float3(39.0f, 49.0f, 59.0f).x, _294.y == float3(39.0f, 49.0f, 59.0f).y, _294.z == float3(39.0f, 49.0f, 59.0f).z)) && all(bool3(_297.x == float3(54.0f, 68.0f, 82.0f).x, _297.y == float3(54.0f, 68.0f, 82.0f).y, _297.z == float3(54.0f, 68.0f, 82.0f).z));
    }
    else
    {
        _301 = false;
    }
    ok = _301;
    return _301;
}

float4 main(float2 _303)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float3x2 _RESERVED_IDENTIFIER_FIXUP_1_splat_4 = float3x2(4.0f.xx, 4.0f.xx, 4.0f.xx);
    float3x2 _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _308 = float2(2.0f, 0.0f) + 4.0f.xx;
    float2 _309 = float2(0.0f, 2.0f) + 4.0f.xx;
    float2 _310 = 0.0f.xx + 4.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(_308, _309, _310);
    bool _322 = false;
    if (true)
    {
        _322 = (all(bool2(_308.x == float2(6.0f, 4.0f).x, _308.y == float2(6.0f, 4.0f).y)) && all(bool2(_309.x == float2(4.0f, 6.0f).x, _309.y == float2(4.0f, 6.0f).y))) && all(bool2(_310.x == 4.0f.xx.x, _310.y == 4.0f.xx.y));
    }
    else
    {
        _322 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _322;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _323 = float2(2.0f, 0.0f) - 4.0f.xx;
    float2 _324 = float2(0.0f, 2.0f) - 4.0f.xx;
    float2 _325 = 0.0f.xx - 4.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(_323, _324, _325);
    bool _337 = false;
    if (_322)
    {
        _337 = (all(bool2(_323.x == float2(-2.0f, -4.0f).x, _323.y == float2(-2.0f, -4.0f).y)) && all(bool2(_324.x == float2(-4.0f, -2.0f).x, _324.y == float2(-4.0f, -2.0f).y))) && all(bool2(_325.x == (-4.0f).xx.x, _325.y == (-4.0f).xx.y));
    }
    else
    {
        _337 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _337;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _338 = float2(2.0f, 0.0f) / 4.0f.xx;
    float2 _339 = float2(0.0f, 2.0f) / 4.0f.xx;
    float2 _340 = 0.0f.xx / 4.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(_338, _339, _340);
    bool _352 = false;
    if (_337)
    {
        _352 = (all(bool2(_338.x == float2(0.5f, 0.0f).x, _338.y == float2(0.5f, 0.0f).y)) && all(bool2(_339.x == float2(0.0f, 0.5f).x, _339.y == float2(0.0f, 0.5f).y))) && all(bool2(_340.x == 0.0f.xx.x, _340.y == 0.0f.xx.y));
    }
    else
    {
        _352 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _352;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_3_splat_4 = float2x3(4.0f.xxx, 4.0f.xxx);
    float2x3 _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _355 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _356 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(_355, _356);
    bool _365 = false;
    if (_352)
    {
        _365 = all(bool3(_355.x == float3(6.0f, 4.0f, 4.0f).x, _355.y == float3(6.0f, 4.0f, 4.0f).y, _355.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_356.x == float3(4.0f, 6.0f, 4.0f).x, _356.y == float3(4.0f, 6.0f, 4.0f).y, _356.z == float3(4.0f, 6.0f, 4.0f).z));
    }
    else
    {
        _365 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _365;
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _366 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _367 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(_366, _367);
    bool _376 = false;
    if (_365)
    {
        _376 = all(bool3(_366.x == float3(2.0f, 4.0f, 4.0f).x, _366.y == float3(2.0f, 4.0f, 4.0f).y, _366.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_367.x == float3(4.0f, 2.0f, 4.0f).x, _367.y == float3(4.0f, 2.0f, 4.0f).y, _367.z == float3(4.0f, 2.0f, 4.0f).z));
    }
    else
    {
        _376 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _376;
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _377 = 4.0f.xxx / 2.0f.xxx;
    float3 _378 = 4.0f.xxx / 2.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(_377, _378);
    bool _387 = false;
    if (_376)
    {
        _387 = all(bool3(_377.x == 2.0f.xxx.x, _377.y == 2.0f.xxx.y, _377.z == 2.0f.xxx.z)) && all(bool3(_378.x == 2.0f.xxx.x, _378.y == 2.0f.xxx.y, _378.z == 2.0f.xxx.z));
    }
    else
    {
        _387 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _387;
    float4x3 _RESERVED_IDENTIFIER_FIXUP_5_m = float4x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), float3(10.0f, 11.0f, 12.0f));
    float3 _389 = float3(1.0f, 2.0f, 3.0f) + float3(16.0f, 15.0f, 14.0f);
    float3 _390 = float3(4.0f, 5.0f, 6.0f) + float3(13.0f, 12.0f, 11.0f);
    float3 _391 = float3(7.0f, 8.0f, 9.0f) + float3(10.0f, 9.0f, 8.0f);
    float3 _392 = float3(10.0f, 11.0f, 12.0f) + float3(7.0f, 6.0f, 5.0f);
    _RESERVED_IDENTIFIER_FIXUP_5_m = float4x3(_389, _390, _391, _392);
    bool _407 = false;
    if (_387)
    {
        _407 = ((all(bool3(_389.x == 17.0f.xxx.x, _389.y == 17.0f.xxx.y, _389.z == 17.0f.xxx.z)) && all(bool3(_390.x == 17.0f.xxx.x, _390.y == 17.0f.xxx.y, _390.z == 17.0f.xxx.z))) && all(bool3(_391.x == 17.0f.xxx.x, _391.y == 17.0f.xxx.y, _391.z == 17.0f.xxx.z))) && all(bool3(_392.x == 17.0f.xxx.x, _392.y == 17.0f.xxx.y, _392.z == 17.0f.xxx.z));
    }
    else
    {
        _407 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _407;
    float4x2 _RESERVED_IDENTIFIER_FIXUP_6_m = float4x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f), float2(50.0f, 60.0f), float2(70.0f, 80.0f));
    float2 _409 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _410 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    float2 _411 = float2(50.0f, 60.0f) - float2(5.0f, 6.0f);
    float2 _412 = float2(70.0f, 80.0f) - float2(7.0f, 8.0f);
    _RESERVED_IDENTIFIER_FIXUP_6_m = float4x2(_409, _410, _411, _412);
    bool _427 = false;
    if (_407)
    {
        _427 = ((all(bool2(_409.x == float2(9.0f, 18.0f).x, _409.y == float2(9.0f, 18.0f).y)) && all(bool2(_410.x == float2(27.0f, 36.0f).x, _410.y == float2(27.0f, 36.0f).y))) && all(bool2(_411.x == float2(45.0f, 54.0f).x, _411.y == float2(45.0f, 54.0f).y))) && all(bool2(_412.x == float2(63.0f, 72.0f).x, _412.y == float2(63.0f, 72.0f).y));
    }
    else
    {
        _427 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _427;
    float2x4 _RESERVED_IDENTIFIER_FIXUP_7_m = float2x4(float4(10.0f, 20.0f, 30.0f, 40.0f), float4(10.0f, 20.0f, 30.0f, 40.0f));
    float4 _429 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 10.0f.xxxx;
    float4 _430 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 5.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_7_m = float2x4(_429, _430);
    bool _439 = false;
    if (_427)
    {
        _439 = all(bool4(_429.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _429.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _429.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _429.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_430.x == float4(2.0f, 4.0f, 6.0f, 8.0f).x, _430.y == float4(2.0f, 4.0f, 6.0f, 8.0f).y, _430.z == float4(2.0f, 4.0f, 6.0f, 8.0f).z, _430.w == float4(2.0f, 4.0f, 6.0f, 8.0f).w));
    }
    else
    {
        _439 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _439;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_8_m = float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f));
    float2x3 _441 = mul(float2x2(float2(1.0f, 4.0f), float2(2.0f, 5.0f)), float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f)));
    _RESERVED_IDENTIFIER_FIXUP_8_m = _441;
    bool _451 = false;
    if (_439)
    {
        float3 _444 = _441[0];
        float3 _447 = _441[1];
        _451 = all(bool3(_444.x == float3(39.0f, 49.0f, 59.0f).x, _444.y == float3(39.0f, 49.0f, 59.0f).y, _444.z == float3(39.0f, 49.0f, 59.0f).z)) && all(bool3(_447.x == float3(54.0f, 68.0f, 82.0f).x, _447.y == float3(54.0f, 68.0f, 82.0f).y, _447.z == float3(54.0f, 68.0f, 82.0f).z));
    }
    else
    {
        _451 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _451;
    bool _455 = false;
    if (_451)
    {
        _455 = test_matrix_op_matrix_half_b();
    }
    else
    {
        _455 = false;
    }
    float4 _456 = 0.0f.xxxx;
    if (_455)
    {
        _456 = _11_colorGreen;
    }
    else
    {
        _456 = _11_colorRed;
    }
    return _456;
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
