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
    float3x3 splat_4 = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3x3 splat_2 = float3x3(2.0f.xxx, 2.0f.xxx, 2.0f.xxx);
    float3x3 m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _47 = float3(2.0f, 0.0f, 0.0f) + 4.0f.xxx;
    float3 _48 = float3(0.0f, 2.0f, 0.0f) + 4.0f.xxx;
    float3 _49 = float3(0.0f, 0.0f, 2.0f) + 4.0f.xxx;
    m = float3x3(_47, _48, _49);
    bool _68 = false;
    if (true)
    {
        _68 = (all(bool3(_47.x == float3(6.0f, 4.0f, 4.0f).x, _47.y == float3(6.0f, 4.0f, 4.0f).y, _47.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_48.x == float3(4.0f, 6.0f, 4.0f).x, _48.y == float3(4.0f, 6.0f, 4.0f).y, _48.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_49.x == float3(4.0f, 4.0f, 6.0f).x, _49.y == float3(4.0f, 4.0f, 6.0f).y, _49.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _68 = false;
    }
    ok = _68;
    m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _69 = float3(2.0f, 0.0f, 0.0f) - 4.0f.xxx;
    float3 _70 = float3(0.0f, 2.0f, 0.0f) - 4.0f.xxx;
    float3 _71 = float3(0.0f, 0.0f, 2.0f) - 4.0f.xxx;
    m = float3x3(_69, _70, _71);
    bool _89 = false;
    if (_68)
    {
        _89 = (all(bool3(_69.x == float3(-2.0f, -4.0f, -4.0f).x, _69.y == float3(-2.0f, -4.0f, -4.0f).y, _69.z == float3(-2.0f, -4.0f, -4.0f).z)) && all(bool3(_70.x == float3(-4.0f, -2.0f, -4.0f).x, _70.y == float3(-4.0f, -2.0f, -4.0f).y, _70.z == float3(-4.0f, -2.0f, -4.0f).z))) && all(bool3(_71.x == float3(-4.0f, -4.0f, -2.0f).x, _71.y == float3(-4.0f, -4.0f, -2.0f).y, _71.z == float3(-4.0f, -4.0f, -2.0f).z));
    }
    else
    {
        _89 = false;
    }
    ok = _89;
    m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _90 = float3(2.0f, 0.0f, 0.0f) / 4.0f.xxx;
    float3 _91 = float3(0.0f, 2.0f, 0.0f) / 4.0f.xxx;
    float3 _92 = float3(0.0f, 0.0f, 2.0f) / 4.0f.xxx;
    m = float3x3(_90, _91, _92);
    bool _109 = false;
    if (_89)
    {
        _109 = (all(bool3(_90.x == float3(0.5f, 0.0f, 0.0f).x, _90.y == float3(0.5f, 0.0f, 0.0f).y, _90.z == float3(0.5f, 0.0f, 0.0f).z)) && all(bool3(_91.x == float3(0.0f, 0.5f, 0.0f).x, _91.y == float3(0.0f, 0.5f, 0.0f).y, _91.z == float3(0.0f, 0.5f, 0.0f).z))) && all(bool3(_92.x == float3(0.0f, 0.0f, 0.5f).x, _92.y == float3(0.0f, 0.0f, 0.5f).y, _92.z == float3(0.0f, 0.0f, 0.5f).z));
    }
    else
    {
        _109 = false;
    }
    ok = _109;
    m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _110 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _111 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    float3 _112 = 4.0f.xxx + float3(0.0f, 0.0f, 2.0f);
    m = float3x3(_110, _111, _112);
    bool _124 = false;
    if (_109)
    {
        _124 = (all(bool3(_110.x == float3(6.0f, 4.0f, 4.0f).x, _110.y == float3(6.0f, 4.0f, 4.0f).y, _110.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_111.x == float3(4.0f, 6.0f, 4.0f).x, _111.y == float3(4.0f, 6.0f, 4.0f).y, _111.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_112.x == float3(4.0f, 4.0f, 6.0f).x, _112.y == float3(4.0f, 4.0f, 6.0f).y, _112.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _124 = false;
    }
    ok = _124;
    m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _125 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _126 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    float3 _127 = 4.0f.xxx - float3(0.0f, 0.0f, 2.0f);
    m = float3x3(_125, _126, _127);
    bool _143 = false;
    if (_124)
    {
        _143 = (all(bool3(_125.x == float3(2.0f, 4.0f, 4.0f).x, _125.y == float3(2.0f, 4.0f, 4.0f).y, _125.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_126.x == float3(4.0f, 2.0f, 4.0f).x, _126.y == float3(4.0f, 2.0f, 4.0f).y, _126.z == float3(4.0f, 2.0f, 4.0f).z))) && all(bool3(_127.x == float3(4.0f, 4.0f, 2.0f).x, _127.y == float3(4.0f, 4.0f, 2.0f).y, _127.z == float3(4.0f, 4.0f, 2.0f).z));
    }
    else
    {
        _143 = false;
    }
    ok = _143;
    m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _144 = 4.0f.xxx / 2.0f.xxx;
    float3 _145 = 4.0f.xxx / 2.0f.xxx;
    float3 _146 = 4.0f.xxx / 2.0f.xxx;
    m = float3x3(_144, _145, _146);
    bool _158 = false;
    if (_143)
    {
        _158 = (all(bool3(_144.x == 2.0f.xxx.x, _144.y == 2.0f.xxx.y, _144.z == 2.0f.xxx.z)) && all(bool3(_145.x == 2.0f.xxx.x, _145.y == 2.0f.xxx.y, _145.z == 2.0f.xxx.z))) && all(bool3(_146.x == 2.0f.xxx.x, _146.y == 2.0f.xxx.y, _146.z == 2.0f.xxx.z));
    }
    else
    {
        _158 = false;
    }
    ok = _158;
    float4x4 m_1 = float4x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f), float4(13.0f, 14.0f, 15.0f, 16.0f));
    float4 _185 = float4(1.0f, 2.0f, 3.0f, 4.0f) + float4(16.0f, 15.0f, 14.0f, 13.0f);
    float4 _186 = float4(5.0f, 6.0f, 7.0f, 8.0f) + float4(12.0f, 11.0f, 10.0f, 9.0f);
    float4 _187 = float4(9.0f, 10.0f, 11.0f, 12.0f) + float4(8.0f, 7.0f, 6.0f, 5.0f);
    float4 _188 = float4(13.0f, 14.0f, 15.0f, 16.0f) + float4(4.0f, 3.0f, 2.0f, 1.0f);
    m_1 = float4x4(_185, _186, _187, _188);
    bool _207 = false;
    if (_158)
    {
        _207 = ((all(bool4(_185.x == 17.0f.xxxx.x, _185.y == 17.0f.xxxx.y, _185.z == 17.0f.xxxx.z, _185.w == 17.0f.xxxx.w)) && all(bool4(_186.x == 17.0f.xxxx.x, _186.y == 17.0f.xxxx.y, _186.z == 17.0f.xxxx.z, _186.w == 17.0f.xxxx.w))) && all(bool4(_187.x == 17.0f.xxxx.x, _187.y == 17.0f.xxxx.y, _187.z == 17.0f.xxxx.z, _187.w == 17.0f.xxxx.w))) && all(bool4(_188.x == 17.0f.xxxx.x, _188.y == 17.0f.xxxx.y, _188.z == 17.0f.xxxx.z, _188.w == 17.0f.xxxx.w));
    }
    else
    {
        _207 = false;
    }
    ok = _207;
    float2x2 m_2 = float2x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f));
    float2 _220 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _221 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    m_2 = float2x2(_220, _221);
    bool _237 = false;
    if (_207)
    {
        _237 = all(bool2(_220.x == float2(9.0f, 18.0f).x, _220.y == float2(9.0f, 18.0f).y)) && all(bool2(_221.x == float2(27.0f, 36.0f).x, _221.y == float2(27.0f, 36.0f).y));
    }
    else
    {
        _237 = false;
    }
    ok = _237;
    float2x2 m_3 = float2x2(float2(2.0f, 4.0f), float2(6.0f, 8.0f));
    float2 _244 = float2(2.0f, 4.0f) / 2.0f.xx;
    float2 _245 = float2(6.0f, 8.0f) / float2(2.0f, 4.0f);
    m_3 = float2x2(_244, _245);
    bool _256 = false;
    if (_237)
    {
        _256 = all(bool2(_244.x == float2(1.0f, 2.0f).x, _244.y == float2(1.0f, 2.0f).y)) && all(bool2(_245.x == float2(3.0f, 2.0f).x, _245.y == float2(3.0f, 2.0f).y));
    }
    else
    {
        _256 = false;
    }
    ok = _256;
    float2x2 m_4 = float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f));
    float2x2 _262 = mul(float2x2(float2(3.0f, 5.0f), float2(3.0f, 2.0f)), float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f)));
    m_4 = _262;
    bool _277 = false;
    if (_256)
    {
        float2 _270 = _262[0];
        float2 _273 = _262[1];
        _277 = all(bool2(_270.x == float2(38.0f, 26.0f).x, _270.y == float2(38.0f, 26.0f).y)) && all(bool2(_273.x == float2(17.0f, 14.0f).x, _273.y == float2(17.0f, 14.0f).y));
    }
    else
    {
        _277 = false;
    }
    ok = _277;
    float3x3 m_5 = float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f));
    float3x3 _287 = mul(float3x3(float3(3.0f, 3.0f, 4.0f), float3(2.0f, 3.0f, 4.0f), float3(4.0f, 9.0f, 2.0f)), float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f)));
    m_5 = _287;
    bool _314 = false;
    if (_277)
    {
        float3 _303 = _287[0];
        float3 _306 = _287[1];
        float3 _310 = _287[2];
        _314 = (all(bool3(_303.x == float3(130.0f, 51.0f, 35.0f).x, _303.y == float3(130.0f, 51.0f, 35.0f).y, _303.z == float3(130.0f, 51.0f, 35.0f).z)) && all(bool3(_306.x == float3(120.0f, 47.0f, 33.0f).x, _306.y == float3(120.0f, 47.0f, 33.0f).y, _306.z == float3(120.0f, 47.0f, 33.0f).z))) && all(bool3(_310.x == float3(240.0f, 73.0f, 45.0f).x, _310.y == float3(240.0f, 73.0f, 45.0f).y, _310.z == float3(240.0f, 73.0f, 45.0f).z));
    }
    else
    {
        _314 = false;
    }
    ok = _314;
    return _314;
}

float4 main(float2 _316)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_1_splat_4 = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3x3 _RESERVED_IDENTIFIER_FIXUP_2_splat_2 = float3x3(2.0f.xxx, 2.0f.xxx, 2.0f.xxx);
    float3x3 _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _322 = float3(2.0f, 0.0f, 0.0f) + 4.0f.xxx;
    float3 _323 = float3(0.0f, 2.0f, 0.0f) + 4.0f.xxx;
    float3 _324 = float3(0.0f, 0.0f, 2.0f) + 4.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_322, _323, _324);
    bool _336 = false;
    if (true)
    {
        _336 = (all(bool3(_322.x == float3(6.0f, 4.0f, 4.0f).x, _322.y == float3(6.0f, 4.0f, 4.0f).y, _322.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_323.x == float3(4.0f, 6.0f, 4.0f).x, _323.y == float3(4.0f, 6.0f, 4.0f).y, _323.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_324.x == float3(4.0f, 4.0f, 6.0f).x, _324.y == float3(4.0f, 4.0f, 6.0f).y, _324.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _336 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _336;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _337 = float3(2.0f, 0.0f, 0.0f) - 4.0f.xxx;
    float3 _338 = float3(0.0f, 2.0f, 0.0f) - 4.0f.xxx;
    float3 _339 = float3(0.0f, 0.0f, 2.0f) - 4.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_337, _338, _339);
    bool _351 = false;
    if (_336)
    {
        _351 = (all(bool3(_337.x == float3(-2.0f, -4.0f, -4.0f).x, _337.y == float3(-2.0f, -4.0f, -4.0f).y, _337.z == float3(-2.0f, -4.0f, -4.0f).z)) && all(bool3(_338.x == float3(-4.0f, -2.0f, -4.0f).x, _338.y == float3(-4.0f, -2.0f, -4.0f).y, _338.z == float3(-4.0f, -2.0f, -4.0f).z))) && all(bool3(_339.x == float3(-4.0f, -4.0f, -2.0f).x, _339.y == float3(-4.0f, -4.0f, -2.0f).y, _339.z == float3(-4.0f, -4.0f, -2.0f).z));
    }
    else
    {
        _351 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _351;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f), float3(0.0f, 0.0f, 2.0f));
    float3 _352 = float3(2.0f, 0.0f, 0.0f) / 4.0f.xxx;
    float3 _353 = float3(0.0f, 2.0f, 0.0f) / 4.0f.xxx;
    float3 _354 = float3(0.0f, 0.0f, 2.0f) / 4.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_352, _353, _354);
    bool _366 = false;
    if (_351)
    {
        _366 = (all(bool3(_352.x == float3(0.5f, 0.0f, 0.0f).x, _352.y == float3(0.5f, 0.0f, 0.0f).y, _352.z == float3(0.5f, 0.0f, 0.0f).z)) && all(bool3(_353.x == float3(0.0f, 0.5f, 0.0f).x, _353.y == float3(0.0f, 0.5f, 0.0f).y, _353.z == float3(0.0f, 0.5f, 0.0f).z))) && all(bool3(_354.x == float3(0.0f, 0.0f, 0.5f).x, _354.y == float3(0.0f, 0.0f, 0.5f).y, _354.z == float3(0.0f, 0.0f, 0.5f).z));
    }
    else
    {
        _366 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _366;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _367 = 4.0f.xxx + float3(2.0f, 0.0f, 0.0f);
    float3 _368 = 4.0f.xxx + float3(0.0f, 2.0f, 0.0f);
    float3 _369 = 4.0f.xxx + float3(0.0f, 0.0f, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_367, _368, _369);
    bool _381 = false;
    if (_366)
    {
        _381 = (all(bool3(_367.x == float3(6.0f, 4.0f, 4.0f).x, _367.y == float3(6.0f, 4.0f, 4.0f).y, _367.z == float3(6.0f, 4.0f, 4.0f).z)) && all(bool3(_368.x == float3(4.0f, 6.0f, 4.0f).x, _368.y == float3(4.0f, 6.0f, 4.0f).y, _368.z == float3(4.0f, 6.0f, 4.0f).z))) && all(bool3(_369.x == float3(4.0f, 4.0f, 6.0f).x, _369.y == float3(4.0f, 4.0f, 6.0f).y, _369.z == float3(4.0f, 4.0f, 6.0f).z));
    }
    else
    {
        _381 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _381;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _382 = 4.0f.xxx - float3(2.0f, 0.0f, 0.0f);
    float3 _383 = 4.0f.xxx - float3(0.0f, 2.0f, 0.0f);
    float3 _384 = 4.0f.xxx - float3(0.0f, 0.0f, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_382, _383, _384);
    bool _396 = false;
    if (_381)
    {
        _396 = (all(bool3(_382.x == float3(2.0f, 4.0f, 4.0f).x, _382.y == float3(2.0f, 4.0f, 4.0f).y, _382.z == float3(2.0f, 4.0f, 4.0f).z)) && all(bool3(_383.x == float3(4.0f, 2.0f, 4.0f).x, _383.y == float3(4.0f, 2.0f, 4.0f).y, _383.z == float3(4.0f, 2.0f, 4.0f).z))) && all(bool3(_384.x == float3(4.0f, 4.0f, 2.0f).x, _384.y == float3(4.0f, 4.0f, 2.0f).y, _384.z == float3(4.0f, 4.0f, 2.0f).z));
    }
    else
    {
        _396 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _396;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(4.0f.xxx, 4.0f.xxx, 4.0f.xxx);
    float3 _397 = 4.0f.xxx / 2.0f.xxx;
    float3 _398 = 4.0f.xxx / 2.0f.xxx;
    float3 _399 = 4.0f.xxx / 2.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_3_m = float3x3(_397, _398, _399);
    bool _411 = false;
    if (_396)
    {
        _411 = (all(bool3(_397.x == 2.0f.xxx.x, _397.y == 2.0f.xxx.y, _397.z == 2.0f.xxx.z)) && all(bool3(_398.x == 2.0f.xxx.x, _398.y == 2.0f.xxx.y, _398.z == 2.0f.xxx.z))) && all(bool3(_399.x == 2.0f.xxx.x, _399.y == 2.0f.xxx.y, _399.z == 2.0f.xxx.z));
    }
    else
    {
        _411 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _411;
    float4x4 _RESERVED_IDENTIFIER_FIXUP_4_m = float4x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f), float4(13.0f, 14.0f, 15.0f, 16.0f));
    float4 _413 = float4(1.0f, 2.0f, 3.0f, 4.0f) + float4(16.0f, 15.0f, 14.0f, 13.0f);
    float4 _414 = float4(5.0f, 6.0f, 7.0f, 8.0f) + float4(12.0f, 11.0f, 10.0f, 9.0f);
    float4 _415 = float4(9.0f, 10.0f, 11.0f, 12.0f) + float4(8.0f, 7.0f, 6.0f, 5.0f);
    float4 _416 = float4(13.0f, 14.0f, 15.0f, 16.0f) + float4(4.0f, 3.0f, 2.0f, 1.0f);
    _RESERVED_IDENTIFIER_FIXUP_4_m = float4x4(_413, _414, _415, _416);
    bool _431 = false;
    if (_411)
    {
        _431 = ((all(bool4(_413.x == 17.0f.xxxx.x, _413.y == 17.0f.xxxx.y, _413.z == 17.0f.xxxx.z, _413.w == 17.0f.xxxx.w)) && all(bool4(_414.x == 17.0f.xxxx.x, _414.y == 17.0f.xxxx.y, _414.z == 17.0f.xxxx.z, _414.w == 17.0f.xxxx.w))) && all(bool4(_415.x == 17.0f.xxxx.x, _415.y == 17.0f.xxxx.y, _415.z == 17.0f.xxxx.z, _415.w == 17.0f.xxxx.w))) && all(bool4(_416.x == 17.0f.xxxx.x, _416.y == 17.0f.xxxx.y, _416.z == 17.0f.xxxx.z, _416.w == 17.0f.xxxx.w));
    }
    else
    {
        _431 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _431;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_5_m = float2x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f));
    float2 _433 = float2(10.0f, 20.0f) - float2(1.0f, 2.0f);
    float2 _434 = float2(30.0f, 40.0f) - float2(3.0f, 4.0f);
    _RESERVED_IDENTIFIER_FIXUP_5_m = float2x2(_433, _434);
    bool _443 = false;
    if (_431)
    {
        _443 = all(bool2(_433.x == float2(9.0f, 18.0f).x, _433.y == float2(9.0f, 18.0f).y)) && all(bool2(_434.x == float2(27.0f, 36.0f).x, _434.y == float2(27.0f, 36.0f).y));
    }
    else
    {
        _443 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _443;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_6_m = float2x2(float2(2.0f, 4.0f), float2(6.0f, 8.0f));
    float2 _445 = float2(2.0f, 4.0f) / 2.0f.xx;
    float2 _446 = float2(6.0f, 8.0f) / float2(2.0f, 4.0f);
    _RESERVED_IDENTIFIER_FIXUP_6_m = float2x2(_445, _446);
    bool _455 = false;
    if (_443)
    {
        _455 = all(bool2(_445.x == float2(1.0f, 2.0f).x, _445.y == float2(1.0f, 2.0f).y)) && all(bool2(_446.x == float2(3.0f, 2.0f).x, _446.y == float2(3.0f, 2.0f).y));
    }
    else
    {
        _455 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _455;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_7_m = float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f));
    float2x2 _457 = mul(float2x2(float2(3.0f, 5.0f), float2(3.0f, 2.0f)), float2x2(float2(1.0f, 2.0f), float2(7.0f, 4.0f)));
    _RESERVED_IDENTIFIER_FIXUP_7_m = _457;
    bool _467 = false;
    if (_455)
    {
        float2 _460 = _457[0];
        float2 _463 = _457[1];
        _467 = all(bool2(_460.x == float2(38.0f, 26.0f).x, _460.y == float2(38.0f, 26.0f).y)) && all(bool2(_463.x == float2(17.0f, 14.0f).x, _463.y == float2(17.0f, 14.0f).y));
    }
    else
    {
        _467 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _467;
    float3x3 _RESERVED_IDENTIFIER_FIXUP_8_m = float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f));
    float3x3 _469 = mul(float3x3(float3(3.0f, 3.0f, 4.0f), float3(2.0f, 3.0f, 4.0f), float3(4.0f, 9.0f, 2.0f)), float3x3(float3(10.0f, 4.0f, 2.0f), float3(20.0f, 5.0f, 3.0f), float3(10.0f, 6.0f, 5.0f)));
    _RESERVED_IDENTIFIER_FIXUP_8_m = _469;
    bool _483 = false;
    if (_467)
    {
        float3 _472 = _469[0];
        float3 _475 = _469[1];
        float3 _479 = _469[2];
        _483 = (all(bool3(_472.x == float3(130.0f, 51.0f, 35.0f).x, _472.y == float3(130.0f, 51.0f, 35.0f).y, _472.z == float3(130.0f, 51.0f, 35.0f).z)) && all(bool3(_475.x == float3(120.0f, 47.0f, 33.0f).x, _475.y == float3(120.0f, 47.0f, 33.0f).y, _475.z == float3(120.0f, 47.0f, 33.0f).z))) && all(bool3(_479.x == float3(240.0f, 73.0f, 45.0f).x, _479.y == float3(240.0f, 73.0f, 45.0f).y, _479.z == float3(240.0f, 73.0f, 45.0f).z));
    }
    else
    {
        _483 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _483;
    bool _487 = false;
    if (_483)
    {
        _487 = test_matrix_op_matrix_half_b();
    }
    else
    {
        _487 = false;
    }
    float4 _488 = 0.0f.xxxx;
    if (_487)
    {
        _488 = _12_colorGreen;
    }
    else
    {
        _488 = _12_colorRed;
    }
    return _488;
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
