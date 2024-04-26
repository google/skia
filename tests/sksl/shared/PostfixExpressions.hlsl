cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    bool ok = true;
    int i = 5;
    int _32 = 5 + 1;
    i = _32;
    bool _39 = false;
    if (true)
    {
        i = _32 + 1;
        _39 = _32 == 6;
    }
    else
    {
        _39 = false;
    }
    ok = _39;
    bool _45 = false;
    if (_39)
    {
        _45 = i == 7;
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
        i = _48 - 1;
        _51 = _48 == 7;
    }
    else
    {
        _51 = false;
    }
    ok = _51;
    bool _56 = false;
    if (_51)
    {
        _56 = i == 6;
    }
    else
    {
        _56 = false;
    }
    ok = _56;
    int _57 = i;
    int _58 = _57 - 1;
    i = _58;
    bool _62 = false;
    if (_56)
    {
        _62 = _58 == 5;
    }
    else
    {
        _62 = false;
    }
    ok = _62;
    float f = 0.5f;
    float _67 = 0.5f + 1.0f;
    f = _67;
    bool _73 = false;
    if (_62)
    {
        f = _67 + 1.0f;
        _73 = _67 == 1.5f;
    }
    else
    {
        _73 = false;
    }
    ok = _73;
    bool _79 = false;
    if (_73)
    {
        _79 = f == 2.5f;
    }
    else
    {
        _79 = false;
    }
    ok = _79;
    bool _85 = false;
    if (_79)
    {
        float _82 = f;
        f = _82 - 1.0f;
        _85 = _82 == 2.5f;
    }
    else
    {
        _85 = false;
    }
    ok = _85;
    bool _90 = false;
    if (_85)
    {
        _90 = f == 1.5f;
    }
    else
    {
        _90 = false;
    }
    ok = _90;
    float _91 = f;
    float _92 = _91 - 1.0f;
    f = _92;
    bool _96 = false;
    if (_90)
    {
        _96 = _92 == 0.5f;
    }
    else
    {
        _96 = false;
    }
    ok = _96;
    float2 f2 = 0.5f.xx;
    f2.x += 1.0f;
    bool _110 = false;
    if (ok)
    {
        float _107 = f2.x;
        f2.x = _107 + 1.0f;
        _110 = _107 == 1.5f;
    }
    else
    {
        _110 = false;
    }
    ok = _110;
    bool _116 = false;
    if (_110)
    {
        _116 = f2.x == 2.5f;
    }
    else
    {
        _116 = false;
    }
    ok = _116;
    bool _123 = false;
    if (_116)
    {
        float _120 = f2.x;
        f2.x = _120 - 1.0f;
        _123 = _120 == 2.5f;
    }
    else
    {
        _123 = false;
    }
    ok = _123;
    bool _129 = false;
    if (_123)
    {
        _129 = f2.x == 1.5f;
    }
    else
    {
        _129 = false;
    }
    ok = _129;
    f2.x -= 1.0f;
    bool _139 = false;
    if (ok)
    {
        _139 = f2.x == 0.5f;
    }
    else
    {
        _139 = false;
    }
    ok = _139;
    float2 _140 = f2;
    float2 _142 = _140 + 1.0f.xx;
    f2 = _142;
    bool _150 = false;
    if (_139)
    {
        f2 = _142 + 1.0f.xx;
        _150 = all(bool2(_142.x == 1.5f.xx.x, _142.y == 1.5f.xx.y));
    }
    else
    {
        _150 = false;
    }
    ok = _150;
    bool _157 = false;
    if (_150)
    {
        _157 = all(bool2(f2.x == 2.5f.xx.x, f2.y == 2.5f.xx.y));
    }
    else
    {
        _157 = false;
    }
    ok = _157;
    bool _164 = false;
    if (_157)
    {
        float2 _160 = f2;
        f2 = _160 - 1.0f.xx;
        _164 = all(bool2(_160.x == 2.5f.xx.x, _160.y == 2.5f.xx.y));
    }
    else
    {
        _164 = false;
    }
    ok = _164;
    bool _170 = false;
    if (_164)
    {
        _170 = all(bool2(f2.x == 1.5f.xx.x, f2.y == 1.5f.xx.y));
    }
    else
    {
        _170 = false;
    }
    ok = _170;
    float2 _171 = f2;
    float2 _172 = _171 - 1.0f.xx;
    f2 = _172;
    bool _177 = false;
    if (_170)
    {
        _177 = all(bool2(_172.x == 0.5f.xx.x, _172.y == 0.5f.xx.y));
    }
    else
    {
        _177 = false;
    }
    ok = _177;
    int4 i4 = int4(7, 8, 9, 10);
    int4 _186 = int4(7, 8, 9, 10) + int4(1, 1, 1, 1);
    i4 = _186;
    bool _195 = false;
    if (_177)
    {
        i4 = _186 + int4(1, 1, 1, 1);
        _195 = all(bool4(_186.x == int4(8, 9, 10, 11).x, _186.y == int4(8, 9, 10, 11).y, _186.z == int4(8, 9, 10, 11).z, _186.w == int4(8, 9, 10, 11).w));
    }
    else
    {
        _195 = false;
    }
    ok = _195;
    bool _203 = false;
    if (_195)
    {
        _203 = all(bool4(i4.x == int4(9, 10, 11, 12).x, i4.y == int4(9, 10, 11, 12).y, i4.z == int4(9, 10, 11, 12).z, i4.w == int4(9, 10, 11, 12).w));
    }
    else
    {
        _203 = false;
    }
    ok = _203;
    bool _210 = false;
    if (_203)
    {
        int4 _206 = i4;
        i4 = _206 - int4(1, 1, 1, 1);
        _210 = all(bool4(_206.x == int4(9, 10, 11, 12).x, _206.y == int4(9, 10, 11, 12).y, _206.z == int4(9, 10, 11, 12).z, _206.w == int4(9, 10, 11, 12).w));
    }
    else
    {
        _210 = false;
    }
    ok = _210;
    bool _216 = false;
    if (_210)
    {
        _216 = all(bool4(i4.x == int4(8, 9, 10, 11).x, i4.y == int4(8, 9, 10, 11).y, i4.z == int4(8, 9, 10, 11).z, i4.w == int4(8, 9, 10, 11).w));
    }
    else
    {
        _216 = false;
    }
    ok = _216;
    int4 _217 = i4;
    int4 _218 = _217 - int4(1, 1, 1, 1);
    i4 = _218;
    bool _223 = false;
    if (_216)
    {
        _223 = all(bool4(_218.x == int4(7, 8, 9, 10).x, _218.y == int4(7, 8, 9, 10).y, _218.z == int4(7, 8, 9, 10).z, _218.w == int4(7, 8, 9, 10).w));
    }
    else
    {
        _223 = false;
    }
    ok = _223;
    float3x3 m3x3 = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
    float3 _242 = float3(1.0f, 2.0f, 3.0f) + 1.0f.xxx;
    float3 _243 = float3(4.0f, 5.0f, 6.0f) + 1.0f.xxx;
    float3 _244 = float3(7.0f, 8.0f, 9.0f) + 1.0f.xxx;
    m3x3 = float3x3(_242, _243, _244);
    bool _266 = false;
    if (_223)
    {
        m3x3 = float3x3(_242 + 1.0f.xxx, _243 + 1.0f.xxx, _244 + 1.0f.xxx);
        _266 = (all(bool3(_242.x == float3(2.0f, 3.0f, 4.0f).x, _242.y == float3(2.0f, 3.0f, 4.0f).y, _242.z == float3(2.0f, 3.0f, 4.0f).z)) && all(bool3(_243.x == float3(5.0f, 6.0f, 7.0f).x, _243.y == float3(5.0f, 6.0f, 7.0f).y, _243.z == float3(5.0f, 6.0f, 7.0f).z))) && all(bool3(_244.x == float3(8.0f, 9.0f, 10.0f).x, _244.y == float3(8.0f, 9.0f, 10.0f).y, _244.z == float3(8.0f, 9.0f, 10.0f).z));
    }
    else
    {
        _266 = false;
    }
    ok = _266;
    bool _286 = false;
    if (_266)
    {
        _286 = (all(bool3(m3x3[0].x == float3(3.0f, 4.0f, 5.0f).x, m3x3[0].y == float3(3.0f, 4.0f, 5.0f).y, m3x3[0].z == float3(3.0f, 4.0f, 5.0f).z)) && all(bool3(m3x3[1].x == float3(6.0f, 7.0f, 8.0f).x, m3x3[1].y == float3(6.0f, 7.0f, 8.0f).y, m3x3[1].z == float3(6.0f, 7.0f, 8.0f).z))) && all(bool3(m3x3[2].x == float3(9.0f, 10.0f, 11.0f).x, m3x3[2].y == float3(9.0f, 10.0f, 11.0f).y, m3x3[2].z == float3(9.0f, 10.0f, 11.0f).z));
    }
    else
    {
        _286 = false;
    }
    ok = _286;
    bool _305 = false;
    if (_286)
    {
        float3x3 _289 = m3x3;
        m3x3 = float3x3(_289[0] - 1.0f.xxx, _289[1] - 1.0f.xxx, _289[2] - 1.0f.xxx);
        _305 = (all(bool3(_289[0].x == float3(3.0f, 4.0f, 5.0f).x, _289[0].y == float3(3.0f, 4.0f, 5.0f).y, _289[0].z == float3(3.0f, 4.0f, 5.0f).z)) && all(bool3(_289[1].x == float3(6.0f, 7.0f, 8.0f).x, _289[1].y == float3(6.0f, 7.0f, 8.0f).y, _289[1].z == float3(6.0f, 7.0f, 8.0f).z))) && all(bool3(_289[2].x == float3(9.0f, 10.0f, 11.0f).x, _289[2].y == float3(9.0f, 10.0f, 11.0f).y, _289[2].z == float3(9.0f, 10.0f, 11.0f).z));
    }
    else
    {
        _305 = false;
    }
    ok = _305;
    bool _320 = false;
    if (_305)
    {
        _320 = (all(bool3(m3x3[0].x == float3(2.0f, 3.0f, 4.0f).x, m3x3[0].y == float3(2.0f, 3.0f, 4.0f).y, m3x3[0].z == float3(2.0f, 3.0f, 4.0f).z)) && all(bool3(m3x3[1].x == float3(5.0f, 6.0f, 7.0f).x, m3x3[1].y == float3(5.0f, 6.0f, 7.0f).y, m3x3[1].z == float3(5.0f, 6.0f, 7.0f).z))) && all(bool3(m3x3[2].x == float3(8.0f, 9.0f, 10.0f).x, m3x3[2].y == float3(8.0f, 9.0f, 10.0f).y, m3x3[2].z == float3(8.0f, 9.0f, 10.0f).z));
    }
    else
    {
        _320 = false;
    }
    ok = _320;
    float3x3 _321 = m3x3;
    float3 _323 = _321[0] - 1.0f.xxx;
    float3 _325 = _321[1] - 1.0f.xxx;
    float3 _327 = _321[2] - 1.0f.xxx;
    m3x3 = float3x3(_323, _325, _327);
    bool _339 = false;
    if (_320)
    {
        _339 = (all(bool3(_323.x == float3(1.0f, 2.0f, 3.0f).x, _323.y == float3(1.0f, 2.0f, 3.0f).y, _323.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_325.x == float3(4.0f, 5.0f, 6.0f).x, _325.y == float3(4.0f, 5.0f, 6.0f).y, _325.z == float3(4.0f, 5.0f, 6.0f).z))) && all(bool3(_327.x == float3(7.0f, 8.0f, 9.0f).x, _327.y == float3(7.0f, 8.0f, 9.0f).y, _327.z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _339 = false;
    }
    ok = _339;
    float4 _340 = 0.0f.xxxx;
    if (_339)
    {
        _340 = _7_colorGreen;
    }
    else
    {
        _340 = _7_colorRed;
    }
    return _340;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
