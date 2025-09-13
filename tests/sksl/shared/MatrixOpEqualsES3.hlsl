cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorRed : packoffset(c0);
    float4 _12_colorGreen : packoffset(c1);
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
    float2 _42 = float2(2.0f, 0.0f) + 4.0f.xx;
    float2 _43 = float2(0.0f, 2.0f) + 4.0f.xx;
    float2 _44 = 0.0f.xx + 4.0f.xx;
    m = float3x2(_42, _43, _44);
    bool _62 = false;
    if (true)
    {
        _62 = (all(bool2(_42.x == float2(6.0f, 4.0f).x, _42.y == float2(6.0f, 4.0f).y)) && all(bool2(_43.x == float2(4.0f, 6.0f).x, _43.y == float2(4.0f, 6.0f).y))) && all(bool2(_44.x == 4.0f.xx.x, _44.y == 4.0f.xx.y));
    }
    else
    {
        _62 = false;
    }
    ok = _62;
    m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _63 = float2(2.0f, 0.0f) - 4.0f.xx;
    float2 _64 = float2(0.0f, 2.0f) - 4.0f.xx;
    float2 _65 = 0.0f.xx - 4.0f.xx;
    m = float3x2(_63, _64, _65);
    bool _83 = false;
    if (_62)
    {
        _83 = (all(bool2(_63.x == float2(-2.0f, -4.0f).x, _63.y == float2(-2.0f, -4.0f).y)) && all(bool2(_64.x == float2(-4.0f, -2.0f).x, _64.y == float2(-4.0f, -2.0f).y))) && all(bool2(_65.x == (-4.0f).xx.x, _65.y == (-4.0f).xx.y));
    }
    else
    {
        _83 = false;
    }
    ok = _83;
    m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _84 = float2(2.0f, 0.0f) / 4.0f.xx;
    float2 _85 = float2(0.0f, 2.0f) / 4.0f.xx;
    float2 _86 = 0.0f.xx / 4.0f.xx;
    m = float3x2(_84, _85, _86);
    bool _102 = false;
    if (_83)
    {
        _102 = (all(bool2(_84.x == float2(0.5f, 0.0f).x, _84.y == float2(0.5f, 0.0f).y)) && all(bool2(_85.x == float2(0.0f, 0.5f).x, _85.y == float2(0.0f, 0.5f).y))) && all(bool2(_86.x == 0.0f.xx.x, _86.y == 0.0f.xx.y));
    }
    else
    {
        _102 = false;
    }
    ok = _102;
    float2x3 splat_4_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float2x3 m_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _113 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _114 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    m_1 = float2x3(_113, _114);
    bool _127 = false;
    if (_102)
    {
        _127 = all(bool3(_113.x == float3(6.0f, 4.0f, 4.0f).x, _113.y == float3(6.0f, 4.0f, 4.0f).y, _113.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_114.x == float3(4.0f, 6.0f, 4.0f).x, _114.y == float3(4.0f, 6.0f, 4.0f).y, _114.z == float3(4.0f, 6.0f, 4.0f).z));
    }
    else
    {
        _127 = false;
    }
    ok = _127;
    m_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _128 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _129 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    m_1 = float2x3(_128, _129);
    bool _141 = false;
    if (_127)
    {
        _141 = all(bool3(_128.x == float3(2.0f, 4.0f, 4.0f).x, _128.y == float3(2.0f, 4.0f, 4.0f).y, _128.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_129.x == float3(4.0f, 2.0f, 4.0f).x, _129.y == float3(4.0f, 2.0f, 4.0f).y, _129.z == float3(4.0f, 2.0f, 4.0f).z));
    }
    else
    {
        _141 = false;
    }
    ok = _141;
    m_1 = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _144 = 4.0f.xxx / 2.0f.xxx;
    float3 _145 = 4.0f.xxx / 2.0f.xxx;
    m_1 = float2x3(_144, _145);
    bool _154 = false;
    if (_141)
    {
        _154 = all(bool3(_144.x == 2.0f.xxx.x, _144.y == 2.0f.xxx.y, _144.z == 2.0f.xxx.z)) && all(bool3(_145.x == 2.0f.xxx.x, _145.y == 2.0f.xxx.y, _145.z == 2.0f.xxx.z));
    }
    else
    {
        _154 = false;
    }
    ok = _154;
    float4x3 m_2 = float4x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), float3(10.0f, 11.0f, 12.0f));
    float3 _181 = float3(1.0f, 2.0f, 3.0f) + float3(16.0f, 15.0f, 14.0f);
    float3 _182 = float3(4.0f, 5.0f, 6.0f) + float3(13.0f, 12.0f, 11.0f);
    float3 _183 = float3(7.0f, 8.0f, 9.0f) + float3(10.0f, 9.0f, 8.0f);
    float3 _184 = float3(10.0f, 11.0f, 12.0f) + float3(7.0f, 6.0f, 5.0f);
    m_2 = float4x3(_181, _182, _183, _184);
    bool _202 = false;
    if (_154)
    {
        _202 = ((all(bool3(_181.x == 17.0f.xxx.x, _181.y == 17.0f.xxx.y, _181.z == 17.0f.xxx.z)) && all(bool3(_182.x == 17.0f.xxx.x, _182.y == 17.0f.xxx.y, _182.z == 17.0f.xxx.z))) && all(bool3(_183.x == 17.0f.xxx.x, _183.y == 17.0f.xxx.y, _183.z == 17.0f.xxx.z))) && all(bool3(_184.x == 17.0f.xxx.x, _184.y == 17.0f.xxx.y, _184.z == 17.0f.xxx.z));
    }
    else
    {
        _202 = false;
    }
    ok = _202;
    float4x2 m_3 = float4x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f), float2(50.0f, 60.0f), float2(70.0f, 80.0f));
    float2 _223 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _224 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    float2 _225 = float2(50.0f, 60.0f) - float2(5.0f, 6.0f);
    float2 _226 = float2(70.0f, 80.0f) - float2(7.0f, 8.0f);
    m_3 = float4x2(_223, _224, _225, _226);
    bool _253 = false;
    if (_202)
    {
        _253 = ((all(bool2(_223.x == float2(9.0f, 18.0f).x, _223.y == float2(9.0f, 18.0f).y)) && all(bool2(_224.x == float2(27.0f, 36.0f).x, _224.y == float2(27.0f, 36.0f).y))) && all(bool2(_225.x == float2(45.0f, 54.0f).x, _225.y == float2(45.0f, 54.0f).y))) && all(bool2(_226.x == float2(63.0f, 72.0f).x, _226.y == float2(63.0f, 72.0f).y));
    }
    else
    {
        _253 = false;
    }
    ok = _253;
    float2x4 m_4 = float2x4(float4(10.0f, 20.0f, 30.0f, 40.0f), float4(10.0f, 20.0f, 30.0f, 40.0f));
    float4 _262 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 10.0f.xxxx;
    float4 _263 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 5.0f.xxxx;
    m_4 = float2x4(_262, _263);
    bool _276 = false;
    if (_253)
    {
        _276 = all(bool4(_262.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _262.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _262.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _262.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_263.x == float4(2.0f, 4.0f, 6.0f, 8.0f).x, _263.y == float4(2.0f, 4.0f, 6.0f, 8.0f).y, _263.z == float4(2.0f, 4.0f, 6.0f, 8.0f).z, _263.w == float4(2.0f, 4.0f, 6.0f, 8.0f).w));
    }
    else
    {
        _276 = false;
    }
    ok = _276;
    float2x3 m_5 = float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f));
    float2x3 _285 = mul(float2x2(float2(1.0f, 4.0f), float2(2.0f, 5.0f)), float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f)));
    m_5 = _285;
    bool _303 = false;
    if (_276)
    {
        float3 _296 = _285[0];
        float3 _299 = _285[1];
        _303 = all(bool3(_296.x == float3(39.0f, 49.0f, 59.0f).x, _296.y == float3(39.0f, 49.0f, 59.0f).y, _296.z == float3(39.0f, 49.0f, 59.0f).z)) && all(bool3(_299.x == float3(54.0f, 68.0f, 82.0f).x, _299.y == float3(54.0f, 68.0f, 82.0f).y, _299.z == float3(54.0f, 68.0f, 82.0f).z));
    }
    else
    {
        _303 = false;
    }
    ok = _303;
    return _303;
}

float4 main(float2 _305)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float3x2 _RESERVED_IDENTIFIER_FIXUP_1_splat_4 = float3x2(4.0f.xx, 4.0f.xx, 4.0f.xx);
    float3x2 _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _310 = float2(2.0f, 0.0f) + 4.0f.xx;
    float2 _311 = float2(0.0f, 2.0f) + 4.0f.xx;
    float2 _312 = 0.0f.xx + 4.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(_310, _311, _312);
    bool _324 = false;
    if (true)
    {
        _324 = (all(bool2(_310.x == float2(6.0f, 4.0f).x, _310.y == float2(6.0f, 4.0f).y)) && all(bool2(_311.x == float2(4.0f, 6.0f).x, _311.y == float2(4.0f, 6.0f).y))) && all(bool2(_312.x == 4.0f.xx.x, _312.y == 4.0f.xx.y));
    }
    else
    {
        _324 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _324;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _325 = float2(2.0f, 0.0f) - 4.0f.xx;
    float2 _326 = float2(0.0f, 2.0f) - 4.0f.xx;
    float2 _327 = 0.0f.xx - 4.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(_325, _326, _327);
    bool _339 = false;
    if (_324)
    {
        _339 = (all(bool2(_325.x == float2(-2.0f, -4.0f).x, _325.y == float2(-2.0f, -4.0f).y)) && all(bool2(_326.x == float2(-4.0f, -2.0f).x, _326.y == float2(-4.0f, -2.0f).y))) && all(bool2(_327.x == (-4.0f).xx.x, _327.y == (-4.0f).xx.y));
    }
    else
    {
        _339 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _339;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), 0.0f.xx);
    float2 _340 = float2(2.0f, 0.0f) / 4.0f.xx;
    float2 _341 = float2(0.0f, 2.0f) / 4.0f.xx;
    float2 _342 = 0.0f.xx / 4.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_m = float3x2(_340, _341, _342);
    bool _354 = false;
    if (_339)
    {
        _354 = (all(bool2(_340.x == float2(0.5f, 0.0f).x, _340.y == float2(0.5f, 0.0f).y)) && all(bool2(_341.x == float2(0.0f, 0.5f).x, _341.y == float2(0.0f, 0.5f).y))) && all(bool2(_342.x == 0.0f.xx.x, _342.y == 0.0f.xx.y));
    }
    else
    {
        _354 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _354;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_3_splat_4 = float2x3(4.0f.xxx, 4.0f.xxx);
    float2x3 _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _357 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _358 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(_357, _358);
    bool _367 = false;
    if (_354)
    {
        _367 = all(bool3(_357.x == float3(6.0f, 4.0f, 4.0f).x, _357.y == float3(6.0f, 4.0f, 4.0f).y, _357.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_358.x == float3(4.0f, 6.0f, 4.0f).x, _358.y == float3(4.0f, 6.0f, 4.0f).y, _358.z == float3(4.0f, 6.0f, 4.0f).z));
    }
    else
    {
        _367 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _367;
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _368 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _369 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(_368, _369);
    bool _378 = false;
    if (_367)
    {
        _378 = all(bool3(_368.x == float3(2.0f, 4.0f, 4.0f).x, _368.y == float3(2.0f, 4.0f, 4.0f).y, _368.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_369.x == float3(4.0f, 2.0f, 4.0f).x, _369.y == float3(4.0f, 2.0f, 4.0f).y, _369.z == float3(4.0f, 2.0f, 4.0f).z));
    }
    else
    {
        _378 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _378;
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(4.0f.xxx, 4.0f.xxx);
    float3 _379 = 4.0f.xxx / 2.0f.xxx;
    float3 _380 = 4.0f.xxx / 2.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_4_m = float2x3(_379, _380);
    bool _389 = false;
    if (_378)
    {
        _389 = all(bool3(_379.x == 2.0f.xxx.x, _379.y == 2.0f.xxx.y, _379.z == 2.0f.xxx.z)) && all(bool3(_380.x == 2.0f.xxx.x, _380.y == 2.0f.xxx.y, _380.z == 2.0f.xxx.z));
    }
    else
    {
        _389 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _389;
    float4x3 _RESERVED_IDENTIFIER_FIXUP_5_m = float4x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), float3(10.0f, 11.0f, 12.0f));
    float3 _391 = float3(1.0f, 2.0f, 3.0f) + float3(16.0f, 15.0f, 14.0f);
    float3 _392 = float3(4.0f, 5.0f, 6.0f) + float3(13.0f, 12.0f, 11.0f);
    float3 _393 = float3(7.0f, 8.0f, 9.0f) + float3(10.0f, 9.0f, 8.0f);
    float3 _394 = float3(10.0f, 11.0f, 12.0f) + float3(7.0f, 6.0f, 5.0f);
    _RESERVED_IDENTIFIER_FIXUP_5_m = float4x3(_391, _392, _393, _394);
    bool _409 = false;
    if (_389)
    {
        _409 = ((all(bool3(_391.x == 17.0f.xxx.x, _391.y == 17.0f.xxx.y, _391.z == 17.0f.xxx.z)) && all(bool3(_392.x == 17.0f.xxx.x, _392.y == 17.0f.xxx.y, _392.z == 17.0f.xxx.z))) && all(bool3(_393.x == 17.0f.xxx.x, _393.y == 17.0f.xxx.y, _393.z == 17.0f.xxx.z))) && all(bool3(_394.x == 17.0f.xxx.x, _394.y == 17.0f.xxx.y, _394.z == 17.0f.xxx.z));
    }
    else
    {
        _409 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _409;
    float4x2 _RESERVED_IDENTIFIER_FIXUP_6_m = float4x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f), float2(50.0f, 60.0f), float2(70.0f, 80.0f));
    float2 _411 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _412 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    float2 _413 = float2(50.0f, 60.0f) - float2(5.0f, 6.0f);
    float2 _414 = float2(70.0f, 80.0f) - float2(7.0f, 8.0f);
    _RESERVED_IDENTIFIER_FIXUP_6_m = float4x2(_411, _412, _413, _414);
    bool _429 = false;
    if (_409)
    {
        _429 = ((all(bool2(_411.x == float2(9.0f, 18.0f).x, _411.y == float2(9.0f, 18.0f).y)) && all(bool2(_412.x == float2(27.0f, 36.0f).x, _412.y == float2(27.0f, 36.0f).y))) && all(bool2(_413.x == float2(45.0f, 54.0f).x, _413.y == float2(45.0f, 54.0f).y))) && all(bool2(_414.x == float2(63.0f, 72.0f).x, _414.y == float2(63.0f, 72.0f).y));
    }
    else
    {
        _429 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _429;
    float2x4 _RESERVED_IDENTIFIER_FIXUP_7_m = float2x4(float4(10.0f, 20.0f, 30.0f, 40.0f), float4(10.0f, 20.0f, 30.0f, 40.0f));
    float4 _431 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 10.0f.xxxx;
    float4 _432 = float4(10.0f, 20.0f, 30.0f, 40.0f) / 5.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_7_m = float2x4(_431, _432);
    bool _441 = false;
    if (_429)
    {
        _441 = all(bool4(_431.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _431.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _431.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _431.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_432.x == float4(2.0f, 4.0f, 6.0f, 8.0f).x, _432.y == float4(2.0f, 4.0f, 6.0f, 8.0f).y, _432.z == float4(2.0f, 4.0f, 6.0f, 8.0f).z, _432.w == float4(2.0f, 4.0f, 6.0f, 8.0f).w));
    }
    else
    {
        _441 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _441;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_8_m = float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f));
    float2x3 _443 = mul(float2x2(float2(1.0f, 4.0f), float2(2.0f, 5.0f)), float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f)));
    _RESERVED_IDENTIFIER_FIXUP_8_m = _443;
    bool _453 = false;
    if (_441)
    {
        float3 _446 = _443[0];
        float3 _449 = _443[1];
        _453 = all(bool3(_446.x == float3(39.0f, 49.0f, 59.0f).x, _446.y == float3(39.0f, 49.0f, 59.0f).y, _446.z == float3(39.0f, 49.0f, 59.0f).z)) && all(bool3(_449.x == float3(54.0f, 68.0f, 82.0f).x, _449.y == float3(54.0f, 68.0f, 82.0f).y, _449.z == float3(54.0f, 68.0f, 82.0f).z));
    }
    else
    {
        _453 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _453;
    bool _457 = false;
    if (_453)
    {
        _457 = test_matrix_op_matrix_half_b();
    }
    else
    {
        _457 = false;
    }
    float4 _458 = 0.0f.xxxx;
    if (_457)
    {
        _458 = _12_colorGreen;
    }
    else
    {
        _458 = _12_colorRed;
    }
    return _458;
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
