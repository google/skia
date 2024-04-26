cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    row_major float2x2 _7_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    bool ok = true;
    int i = 5;
    int _33 = 5 + 1;
    i = _33;
    bool _39 = false;
    if (true)
    {
        _39 = _33 == 6;
    }
    else
    {
        _39 = false;
    }
    ok = _39;
    bool _45 = false;
    if (_39)
    {
        int _42 = _33 + 1;
        i = _42;
        _45 = _42 == 7;
    }
    else
    {
        _45 = false;
    }
    ok = _45;
    bool _51 = false;
    if (_45)
    {
        int _48 = i;
        int _49 = _48 - 1;
        i = _49;
        _51 = _49 == 6;
    }
    else
    {
        _51 = false;
    }
    ok = _51;
    int _52 = i;
    int _53 = _52 - 1;
    i = _53;
    bool _57 = false;
    if (_51)
    {
        _57 = _53 == 5;
    }
    else
    {
        _57 = false;
    }
    ok = _57;
    float f = 0.5f;
    float _62 = 0.5f + 1.0f;
    f = _62;
    bool _67 = false;
    if (_57)
    {
        _67 = _62 == 1.5f;
    }
    else
    {
        _67 = false;
    }
    ok = _67;
    bool _73 = false;
    if (_67)
    {
        float _70 = _62 + 1.0f;
        f = _70;
        _73 = _70 == 2.5f;
    }
    else
    {
        _73 = false;
    }
    ok = _73;
    bool _79 = false;
    if (_73)
    {
        float _76 = f;
        float _77 = _76 - 1.0f;
        f = _77;
        _79 = _77 == 1.5f;
    }
    else
    {
        _79 = false;
    }
    ok = _79;
    float _80 = f;
    float _81 = _80 - 1.0f;
    f = _81;
    bool _85 = false;
    if (_79)
    {
        _85 = _81 == 0.5f;
    }
    else
    {
        _85 = false;
    }
    ok = _85;
    float2 f2 = 0.5f.xx;
    f2.x += 1.0f;
    bool _98 = false;
    if (ok)
    {
        _98 = f2.x == 1.5f;
    }
    else
    {
        _98 = false;
    }
    ok = _98;
    bool _105 = false;
    if (_98)
    {
        float _102 = f2.x;
        float _103 = _102 + 1.0f;
        f2.x = _103;
        _105 = _103 == 2.5f;
    }
    else
    {
        _105 = false;
    }
    ok = _105;
    bool _112 = false;
    if (_105)
    {
        float _109 = f2.x;
        float _110 = _109 - 1.0f;
        f2.x = _110;
        _112 = _110 == 1.5f;
    }
    else
    {
        _112 = false;
    }
    ok = _112;
    f2.x -= 1.0f;
    bool _122 = false;
    if (ok)
    {
        _122 = f2.x == 0.5f;
    }
    else
    {
        _122 = false;
    }
    ok = _122;
    float2 _124 = f2;
    float2 _125 = _124 + 1.0f.xx;
    f2 = _125;
    bool _132 = false;
    if (_122)
    {
        _132 = all(bool2(_125.x == 1.5f.xx.x, _125.y == 1.5f.xx.y));
    }
    else
    {
        _132 = false;
    }
    ok = _132;
    bool _139 = false;
    if (_132)
    {
        float2 _135 = _125 + 1.0f.xx;
        f2 = _135;
        _139 = all(bool2(_135.x == 2.5f.xx.x, _135.y == 2.5f.xx.y));
    }
    else
    {
        _139 = false;
    }
    ok = _139;
    bool _146 = false;
    if (_139)
    {
        float2 _142 = f2;
        float2 _143 = _142 - 1.0f.xx;
        f2 = _143;
        _146 = all(bool2(_143.x == 1.5f.xx.x, _143.y == 1.5f.xx.y));
    }
    else
    {
        _146 = false;
    }
    ok = _146;
    float2 _147 = f2;
    float2 _148 = _147 - 1.0f.xx;
    f2 = _148;
    bool _153 = false;
    if (_146)
    {
        _153 = all(bool2(_148.x == 0.5f.xx.x, _148.y == 0.5f.xx.y));
    }
    else
    {
        _153 = false;
    }
    ok = _153;
    int4 i4 = int4(7, 8, 9, 10);
    int4 _162 = int4(7, 8, 9, 10) + int4(1, 1, 1, 1);
    i4 = _162;
    bool _170 = false;
    if (_153)
    {
        _170 = all(bool4(_162.x == int4(8, 9, 10, 11).x, _162.y == int4(8, 9, 10, 11).y, _162.z == int4(8, 9, 10, 11).z, _162.w == int4(8, 9, 10, 11).w));
    }
    else
    {
        _170 = false;
    }
    ok = _170;
    bool _178 = false;
    if (_170)
    {
        int4 _173 = _162 + int4(1, 1, 1, 1);
        i4 = _173;
        _178 = all(bool4(_173.x == int4(9, 10, 11, 12).x, _173.y == int4(9, 10, 11, 12).y, _173.z == int4(9, 10, 11, 12).z, _173.w == int4(9, 10, 11, 12).w));
    }
    else
    {
        _178 = false;
    }
    ok = _178;
    bool _185 = false;
    if (_178)
    {
        int4 _181 = i4;
        int4 _182 = _181 - int4(1, 1, 1, 1);
        i4 = _182;
        _185 = all(bool4(_182.x == int4(8, 9, 10, 11).x, _182.y == int4(8, 9, 10, 11).y, _182.z == int4(8, 9, 10, 11).z, _182.w == int4(8, 9, 10, 11).w));
    }
    else
    {
        _185 = false;
    }
    ok = _185;
    int4 _186 = i4;
    int4 _187 = _186 - int4(1, 1, 1, 1);
    i4 = _187;
    bool _192 = false;
    if (_185)
    {
        _192 = all(bool4(_187.x == int4(7, 8, 9, 10).x, _187.y == int4(7, 8, 9, 10).y, _187.z == int4(7, 8, 9, 10).z, _187.w == int4(7, 8, 9, 10).w));
    }
    else
    {
        _192 = false;
    }
    ok = _192;
    float3x3 m3x3 = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
    float3 _211 = float3(1.0f, 2.0f, 3.0f) + 1.0f.xxx;
    float3 _212 = float3(4.0f, 5.0f, 6.0f) + 1.0f.xxx;
    float3 _213 = float3(7.0f, 8.0f, 9.0f) + 1.0f.xxx;
    m3x3 = float3x3(_211, _212, _213);
    bool _231 = false;
    if (_192)
    {
        _231 = (all(bool3(_211.x == float3(2.0f, 3.0f, 4.0f).x, _211.y == float3(2.0f, 3.0f, 4.0f).y, _211.z == float3(2.0f, 3.0f, 4.0f).z)) && all(bool3(_212.x == float3(5.0f, 6.0f, 7.0f).x, _212.y == float3(5.0f, 6.0f, 7.0f).y, _212.z == float3(5.0f, 6.0f, 7.0f).z))) && all(bool3(_213.x == float3(8.0f, 9.0f, 10.0f).x, _213.y == float3(8.0f, 9.0f, 10.0f).y, _213.z == float3(8.0f, 9.0f, 10.0f).z));
    }
    else
    {
        _231 = false;
    }
    ok = _231;
    bool _251 = false;
    if (_231)
    {
        float3 _234 = _211 + 1.0f.xxx;
        float3 _235 = _212 + 1.0f.xxx;
        float3 _236 = _213 + 1.0f.xxx;
        m3x3 = float3x3(_234, _235, _236);
        _251 = (all(bool3(_234.x == float3(3.0f, 4.0f, 5.0f).x, _234.y == float3(3.0f, 4.0f, 5.0f).y, _234.z == float3(3.0f, 4.0f, 5.0f).z)) && all(bool3(_235.x == float3(6.0f, 7.0f, 8.0f).x, _235.y == float3(6.0f, 7.0f, 8.0f).y, _235.z == float3(6.0f, 7.0f, 8.0f).z))) && all(bool3(_236.x == float3(9.0f, 10.0f, 11.0f).x, _236.y == float3(9.0f, 10.0f, 11.0f).y, _236.z == float3(9.0f, 10.0f, 11.0f).z));
    }
    else
    {
        _251 = false;
    }
    ok = _251;
    bool _270 = false;
    if (_251)
    {
        float3x3 _254 = m3x3;
        float3 _256 = _254[0] - 1.0f.xxx;
        float3 _258 = _254[1] - 1.0f.xxx;
        float3 _260 = _254[2] - 1.0f.xxx;
        m3x3 = float3x3(_256, _258, _260);
        _270 = (all(bool3(_256.x == float3(2.0f, 3.0f, 4.0f).x, _256.y == float3(2.0f, 3.0f, 4.0f).y, _256.z == float3(2.0f, 3.0f, 4.0f).z)) && all(bool3(_258.x == float3(5.0f, 6.0f, 7.0f).x, _258.y == float3(5.0f, 6.0f, 7.0f).y, _258.z == float3(5.0f, 6.0f, 7.0f).z))) && all(bool3(_260.x == float3(8.0f, 9.0f, 10.0f).x, _260.y == float3(8.0f, 9.0f, 10.0f).y, _260.z == float3(8.0f, 9.0f, 10.0f).z));
    }
    else
    {
        _270 = false;
    }
    ok = _270;
    float3x3 _271 = m3x3;
    float3 _273 = _271[0] - 1.0f.xxx;
    float3 _275 = _271[1] - 1.0f.xxx;
    float3 _277 = _271[2] - 1.0f.xxx;
    m3x3 = float3x3(_273, _275, _277);
    bool _289 = false;
    if (_270)
    {
        _289 = (all(bool3(_273.x == float3(1.0f, 2.0f, 3.0f).x, _273.y == float3(1.0f, 2.0f, 3.0f).y, _273.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_275.x == float3(4.0f, 5.0f, 6.0f).x, _275.y == float3(4.0f, 5.0f, 6.0f).y, _275.z == float3(4.0f, 5.0f, 6.0f).z))) && all(bool3(_277.x == float3(7.0f, 8.0f, 9.0f).x, _277.y == float3(7.0f, 8.0f, 9.0f).y, _277.z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _289 = false;
    }
    ok = _289;
    bool _297 = false;
    if (_289)
    {
        _297 = _7_colorGreen.x != 1.0f;
    }
    else
    {
        _297 = false;
    }
    ok = _297;
    bool _306 = false;
    if (_297)
    {
        _306 = (-1.0f) == (-_7_colorGreen.y);
    }
    else
    {
        _306 = false;
    }
    ok = _306;
    bool _315 = false;
    if (_306)
    {
        float4 _312 = -_7_colorGreen;
        _315 = all(bool4(float4(0.0f, -1.0f, 0.0f, -1.0f).x == _312.x, float4(0.0f, -1.0f, 0.0f, -1.0f).y == _312.y, float4(0.0f, -1.0f, 0.0f, -1.0f).z == _312.z, float4(0.0f, -1.0f, 0.0f, -1.0f).w == _312.w));
    }
    else
    {
        _315 = false;
    }
    ok = _315;
    bool _338 = false;
    if (_315)
    {
        float2 _329 = -_7_testMatrix2x2[0];
        float2 _331 = -_7_testMatrix2x2[1];
        _338 = all(bool2(float2(-1.0f, -2.0f).x == _329.x, float2(-1.0f, -2.0f).y == _329.y)) && all(bool2(float2(-3.0f, -4.0f).x == _331.x, float2(-3.0f, -4.0f).y == _331.y));
    }
    else
    {
        _338 = false;
    }
    ok = _338;
    int2 _345 = int2(i, -i);
    int2 iv = _345;
    bool _352 = false;
    if (_338)
    {
        _352 = (-i) == (-5);
    }
    else
    {
        _352 = false;
    }
    ok = _352;
    bool _359 = false;
    if (_352)
    {
        int2 _355 = -_345;
        _359 = all(bool2(_355.x == int2(-5, 5).x, _355.y == int2(-5, 5).y));
    }
    else
    {
        _359 = false;
    }
    ok = _359;
    float4 _360 = 0.0f.xxxx;
    if (_359)
    {
        _360 = _7_colorGreen;
    }
    else
    {
        _360 = _7_colorRed;
    }
    return _360;
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
