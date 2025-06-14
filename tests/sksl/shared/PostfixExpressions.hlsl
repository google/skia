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

float4 main(float2 _25)
{
    bool ok = true;
    int i = 5;
    int _35 = 5 + 1;
    i = _35;
    bool _42 = false;
    if (true)
    {
        i = _35 + 1;
        _42 = _35 == 6;
    }
    else
    {
        _42 = false;
    }
    ok = _42;
    bool _48 = false;
    if (_42)
    {
        _48 = i == 7;
    }
    else
    {
        _48 = false;
    }
    ok = _48;
    bool _54 = false;
    if (_48)
    {
        int _51 = i;
        i = _51 - 1;
        _54 = _51 == 7;
    }
    else
    {
        _54 = false;
    }
    ok = _54;
    bool _59 = false;
    if (_54)
    {
        _59 = i == 6;
    }
    else
    {
        _59 = false;
    }
    ok = _59;
    int _60 = i;
    int _61 = _60 - 1;
    i = _61;
    bool _65 = false;
    if (_59)
    {
        _65 = _61 == 5;
    }
    else
    {
        _65 = false;
    }
    ok = _65;
    float f = 0.5f;
    float _70 = 0.5f + 1.0f;
    f = _70;
    bool _76 = false;
    if (_65)
    {
        f = _70 + 1.0f;
        _76 = _70 == 1.5f;
    }
    else
    {
        _76 = false;
    }
    ok = _76;
    bool _82 = false;
    if (_76)
    {
        _82 = f == 2.5f;
    }
    else
    {
        _82 = false;
    }
    ok = _82;
    bool _88 = false;
    if (_82)
    {
        float _85 = f;
        f = _85 - 1.0f;
        _88 = _85 == 2.5f;
    }
    else
    {
        _88 = false;
    }
    ok = _88;
    bool _93 = false;
    if (_88)
    {
        _93 = f == 1.5f;
    }
    else
    {
        _93 = false;
    }
    ok = _93;
    float _94 = f;
    float _95 = _94 - 1.0f;
    f = _95;
    bool _99 = false;
    if (_93)
    {
        _99 = _95 == 0.5f;
    }
    else
    {
        _99 = false;
    }
    ok = _99;
    float2 f2 = 0.5f.xx;
    f2.x += 1.0f;
    bool _113 = false;
    if (ok)
    {
        float _110 = f2.x;
        f2.x = _110 + 1.0f;
        _113 = _110 == 1.5f;
    }
    else
    {
        _113 = false;
    }
    ok = _113;
    bool _119 = false;
    if (_113)
    {
        _119 = f2.x == 2.5f;
    }
    else
    {
        _119 = false;
    }
    ok = _119;
    bool _126 = false;
    if (_119)
    {
        float _123 = f2.x;
        f2.x = _123 - 1.0f;
        _126 = _123 == 2.5f;
    }
    else
    {
        _126 = false;
    }
    ok = _126;
    bool _132 = false;
    if (_126)
    {
        _132 = f2.x == 1.5f;
    }
    else
    {
        _132 = false;
    }
    ok = _132;
    f2.x -= 1.0f;
    bool _142 = false;
    if (ok)
    {
        _142 = f2.x == 0.5f;
    }
    else
    {
        _142 = false;
    }
    ok = _142;
    float2 _143 = f2;
    float2 _145 = _143 + 1.0f.xx;
    f2 = _145;
    bool _153 = false;
    if (_142)
    {
        f2 = _145 + 1.0f.xx;
        _153 = all(bool2(_145.x == 1.5f.xx.x, _145.y == 1.5f.xx.y));
    }
    else
    {
        _153 = false;
    }
    ok = _153;
    bool _160 = false;
    if (_153)
    {
        _160 = all(bool2(f2.x == 2.5f.xx.x, f2.y == 2.5f.xx.y));
    }
    else
    {
        _160 = false;
    }
    ok = _160;
    bool _167 = false;
    if (_160)
    {
        float2 _163 = f2;
        f2 = _163 - 1.0f.xx;
        _167 = all(bool2(_163.x == 2.5f.xx.x, _163.y == 2.5f.xx.y));
    }
    else
    {
        _167 = false;
    }
    ok = _167;
    bool _173 = false;
    if (_167)
    {
        _173 = all(bool2(f2.x == 1.5f.xx.x, f2.y == 1.5f.xx.y));
    }
    else
    {
        _173 = false;
    }
    ok = _173;
    float2 _174 = f2;
    float2 _175 = _174 - 1.0f.xx;
    f2 = _175;
    bool _180 = false;
    if (_173)
    {
        _180 = all(bool2(_175.x == 0.5f.xx.x, _175.y == 0.5f.xx.y));
    }
    else
    {
        _180 = false;
    }
    ok = _180;
    int4 i4 = int4(7, 8, 9, 10);
    int4 _189 = int4(7, 8, 9, 10) + int4(1, 1, 1, 1);
    i4 = _189;
    bool _198 = false;
    if (_180)
    {
        i4 = _189 + int4(1, 1, 1, 1);
        _198 = all(bool4(_189.x == int4(8, 9, 10, 11).x, _189.y == int4(8, 9, 10, 11).y, _189.z == int4(8, 9, 10, 11).z, _189.w == int4(8, 9, 10, 11).w));
    }
    else
    {
        _198 = false;
    }
    ok = _198;
    bool _206 = false;
    if (_198)
    {
        _206 = all(bool4(i4.x == int4(9, 10, 11, 12).x, i4.y == int4(9, 10, 11, 12).y, i4.z == int4(9, 10, 11, 12).z, i4.w == int4(9, 10, 11, 12).w));
    }
    else
    {
        _206 = false;
    }
    ok = _206;
    bool _213 = false;
    if (_206)
    {
        int4 _209 = i4;
        i4 = _209 - int4(1, 1, 1, 1);
        _213 = all(bool4(_209.x == int4(9, 10, 11, 12).x, _209.y == int4(9, 10, 11, 12).y, _209.z == int4(9, 10, 11, 12).z, _209.w == int4(9, 10, 11, 12).w));
    }
    else
    {
        _213 = false;
    }
    ok = _213;
    bool _219 = false;
    if (_213)
    {
        _219 = all(bool4(i4.x == int4(8, 9, 10, 11).x, i4.y == int4(8, 9, 10, 11).y, i4.z == int4(8, 9, 10, 11).z, i4.w == int4(8, 9, 10, 11).w));
    }
    else
    {
        _219 = false;
    }
    ok = _219;
    int4 _220 = i4;
    int4 _221 = _220 - int4(1, 1, 1, 1);
    i4 = _221;
    bool _226 = false;
    if (_219)
    {
        _226 = all(bool4(_221.x == int4(7, 8, 9, 10).x, _221.y == int4(7, 8, 9, 10).y, _221.z == int4(7, 8, 9, 10).z, _221.w == int4(7, 8, 9, 10).w));
    }
    else
    {
        _226 = false;
    }
    ok = _226;
    float3x3 m3x3 = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
    float3 _245 = float3(1.0f, 2.0f, 3.0f) + 1.0f.xxx;
    float3 _246 = float3(4.0f, 5.0f, 6.0f) + 1.0f.xxx;
    float3 _247 = float3(7.0f, 8.0f, 9.0f) + 1.0f.xxx;
    m3x3 = float3x3(_245, _246, _247);
    bool _269 = false;
    if (_226)
    {
        m3x3 = float3x3(_245 + 1.0f.xxx, _246 + 1.0f.xxx, _247 + 1.0f.xxx);
        _269 = (all(bool3(_245.x == float3(2.0f, 3.0f, 4.0f).x, _245.y == float3(2.0f, 3.0f, 4.0f).y, _245.z == float3(2.0f, 3.0f, 4.0f).z)) && all(bool3(_246.x == float3(5.0f, 6.0f, 7.0f).x, _246.y == float3(5.0f, 6.0f, 7.0f).y, _246.z == float3(5.0f, 6.0f, 7.0f).z))) && all(bool3(_247.x == float3(8.0f, 9.0f, 10.0f).x, _247.y == float3(8.0f, 9.0f, 10.0f).y, _247.z == float3(8.0f, 9.0f, 10.0f).z));
    }
    else
    {
        _269 = false;
    }
    ok = _269;
    bool _289 = false;
    if (_269)
    {
        _289 = (all(bool3(m3x3[0].x == float3(3.0f, 4.0f, 5.0f).x, m3x3[0].y == float3(3.0f, 4.0f, 5.0f).y, m3x3[0].z == float3(3.0f, 4.0f, 5.0f).z)) && all(bool3(m3x3[1].x == float3(6.0f, 7.0f, 8.0f).x, m3x3[1].y == float3(6.0f, 7.0f, 8.0f).y, m3x3[1].z == float3(6.0f, 7.0f, 8.0f).z))) && all(bool3(m3x3[2].x == float3(9.0f, 10.0f, 11.0f).x, m3x3[2].y == float3(9.0f, 10.0f, 11.0f).y, m3x3[2].z == float3(9.0f, 10.0f, 11.0f).z));
    }
    else
    {
        _289 = false;
    }
    ok = _289;
    bool _308 = false;
    if (_289)
    {
        float3x3 _292 = m3x3;
        m3x3 = float3x3(_292[0] - 1.0f.xxx, _292[1] - 1.0f.xxx, _292[2] - 1.0f.xxx);
        _308 = (all(bool3(_292[0].x == float3(3.0f, 4.0f, 5.0f).x, _292[0].y == float3(3.0f, 4.0f, 5.0f).y, _292[0].z == float3(3.0f, 4.0f, 5.0f).z)) && all(bool3(_292[1].x == float3(6.0f, 7.0f, 8.0f).x, _292[1].y == float3(6.0f, 7.0f, 8.0f).y, _292[1].z == float3(6.0f, 7.0f, 8.0f).z))) && all(bool3(_292[2].x == float3(9.0f, 10.0f, 11.0f).x, _292[2].y == float3(9.0f, 10.0f, 11.0f).y, _292[2].z == float3(9.0f, 10.0f, 11.0f).z));
    }
    else
    {
        _308 = false;
    }
    ok = _308;
    bool _323 = false;
    if (_308)
    {
        _323 = (all(bool3(m3x3[0].x == float3(2.0f, 3.0f, 4.0f).x, m3x3[0].y == float3(2.0f, 3.0f, 4.0f).y, m3x3[0].z == float3(2.0f, 3.0f, 4.0f).z)) && all(bool3(m3x3[1].x == float3(5.0f, 6.0f, 7.0f).x, m3x3[1].y == float3(5.0f, 6.0f, 7.0f).y, m3x3[1].z == float3(5.0f, 6.0f, 7.0f).z))) && all(bool3(m3x3[2].x == float3(8.0f, 9.0f, 10.0f).x, m3x3[2].y == float3(8.0f, 9.0f, 10.0f).y, m3x3[2].z == float3(8.0f, 9.0f, 10.0f).z));
    }
    else
    {
        _323 = false;
    }
    ok = _323;
    float3x3 _324 = m3x3;
    float3 _326 = _324[0] - 1.0f.xxx;
    float3 _328 = _324[1] - 1.0f.xxx;
    float3 _330 = _324[2] - 1.0f.xxx;
    m3x3 = float3x3(_326, _328, _330);
    bool _342 = false;
    if (_323)
    {
        _342 = (all(bool3(_326.x == float3(1.0f, 2.0f, 3.0f).x, _326.y == float3(1.0f, 2.0f, 3.0f).y, _326.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_328.x == float3(4.0f, 5.0f, 6.0f).x, _328.y == float3(4.0f, 5.0f, 6.0f).y, _328.z == float3(4.0f, 5.0f, 6.0f).z))) && all(bool3(_330.x == float3(7.0f, 8.0f, 9.0f).x, _330.y == float3(7.0f, 8.0f, 9.0f).y, _330.z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _342 = false;
    }
    ok = _342;
    float4 _343 = 0.0f.xxxx;
    if (_342)
    {
        _343 = _11_colorGreen;
    }
    else
    {
        _343 = _11_colorRed;
    }
    return _343;
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
