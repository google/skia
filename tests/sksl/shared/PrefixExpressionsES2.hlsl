cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    row_major float2x2 _11_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    bool ok = true;
    int i = 5;
    int _36 = 5 + 1;
    i = _36;
    bool _42 = false;
    if (true)
    {
        _42 = _36 == 6;
    }
    else
    {
        _42 = false;
    }
    ok = _42;
    bool _48 = false;
    if (_42)
    {
        int _45 = _36 + 1;
        i = _45;
        _48 = _45 == 7;
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
        int _52 = _51 - 1;
        i = _52;
        _54 = _52 == 6;
    }
    else
    {
        _54 = false;
    }
    ok = _54;
    int _55 = i;
    int _56 = _55 - 1;
    i = _56;
    bool _60 = false;
    if (_54)
    {
        _60 = _56 == 5;
    }
    else
    {
        _60 = false;
    }
    ok = _60;
    float f = 0.5f;
    float _65 = 0.5f + 1.0f;
    f = _65;
    bool _70 = false;
    if (_60)
    {
        _70 = _65 == 1.5f;
    }
    else
    {
        _70 = false;
    }
    ok = _70;
    bool _76 = false;
    if (_70)
    {
        float _73 = _65 + 1.0f;
        f = _73;
        _76 = _73 == 2.5f;
    }
    else
    {
        _76 = false;
    }
    ok = _76;
    bool _82 = false;
    if (_76)
    {
        float _79 = f;
        float _80 = _79 - 1.0f;
        f = _80;
        _82 = _80 == 1.5f;
    }
    else
    {
        _82 = false;
    }
    ok = _82;
    float _83 = f;
    float _84 = _83 - 1.0f;
    f = _84;
    bool _88 = false;
    if (_82)
    {
        _88 = _84 == 0.5f;
    }
    else
    {
        _88 = false;
    }
    ok = _88;
    float2 f2 = 0.5f.xx;
    f2.x += 1.0f;
    bool _101 = false;
    if (ok)
    {
        _101 = f2.x == 1.5f;
    }
    else
    {
        _101 = false;
    }
    ok = _101;
    bool _108 = false;
    if (_101)
    {
        float _105 = f2.x;
        float _106 = _105 + 1.0f;
        f2.x = _106;
        _108 = _106 == 2.5f;
    }
    else
    {
        _108 = false;
    }
    ok = _108;
    bool _115 = false;
    if (_108)
    {
        float _112 = f2.x;
        float _113 = _112 - 1.0f;
        f2.x = _113;
        _115 = _113 == 1.5f;
    }
    else
    {
        _115 = false;
    }
    ok = _115;
    f2.x -= 1.0f;
    bool _125 = false;
    if (ok)
    {
        _125 = f2.x == 0.5f;
    }
    else
    {
        _125 = false;
    }
    ok = _125;
    float2 _127 = f2;
    float2 _128 = _127 + 1.0f.xx;
    f2 = _128;
    bool _135 = false;
    if (_125)
    {
        _135 = all(bool2(_128.x == 1.5f.xx.x, _128.y == 1.5f.xx.y));
    }
    else
    {
        _135 = false;
    }
    ok = _135;
    bool _142 = false;
    if (_135)
    {
        float2 _138 = _128 + 1.0f.xx;
        f2 = _138;
        _142 = all(bool2(_138.x == 2.5f.xx.x, _138.y == 2.5f.xx.y));
    }
    else
    {
        _142 = false;
    }
    ok = _142;
    bool _149 = false;
    if (_142)
    {
        float2 _145 = f2;
        float2 _146 = _145 - 1.0f.xx;
        f2 = _146;
        _149 = all(bool2(_146.x == 1.5f.xx.x, _146.y == 1.5f.xx.y));
    }
    else
    {
        _149 = false;
    }
    ok = _149;
    float2 _150 = f2;
    float2 _151 = _150 - 1.0f.xx;
    f2 = _151;
    bool _156 = false;
    if (_149)
    {
        _156 = all(bool2(_151.x == 0.5f.xx.x, _151.y == 0.5f.xx.y));
    }
    else
    {
        _156 = false;
    }
    ok = _156;
    int4 i4 = int4(7, 8, 9, 10);
    int4 _165 = int4(7, 8, 9, 10) + int4(1, 1, 1, 1);
    i4 = _165;
    bool _173 = false;
    if (_156)
    {
        _173 = all(bool4(_165.x == int4(8, 9, 10, 11).x, _165.y == int4(8, 9, 10, 11).y, _165.z == int4(8, 9, 10, 11).z, _165.w == int4(8, 9, 10, 11).w));
    }
    else
    {
        _173 = false;
    }
    ok = _173;
    bool _181 = false;
    if (_173)
    {
        int4 _176 = _165 + int4(1, 1, 1, 1);
        i4 = _176;
        _181 = all(bool4(_176.x == int4(9, 10, 11, 12).x, _176.y == int4(9, 10, 11, 12).y, _176.z == int4(9, 10, 11, 12).z, _176.w == int4(9, 10, 11, 12).w));
    }
    else
    {
        _181 = false;
    }
    ok = _181;
    bool _188 = false;
    if (_181)
    {
        int4 _184 = i4;
        int4 _185 = _184 - int4(1, 1, 1, 1);
        i4 = _185;
        _188 = all(bool4(_185.x == int4(8, 9, 10, 11).x, _185.y == int4(8, 9, 10, 11).y, _185.z == int4(8, 9, 10, 11).z, _185.w == int4(8, 9, 10, 11).w));
    }
    else
    {
        _188 = false;
    }
    ok = _188;
    int4 _189 = i4;
    int4 _190 = _189 - int4(1, 1, 1, 1);
    i4 = _190;
    bool _195 = false;
    if (_188)
    {
        _195 = all(bool4(_190.x == int4(7, 8, 9, 10).x, _190.y == int4(7, 8, 9, 10).y, _190.z == int4(7, 8, 9, 10).z, _190.w == int4(7, 8, 9, 10).w));
    }
    else
    {
        _195 = false;
    }
    ok = _195;
    float3x3 m3x3 = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
    float3 _214 = float3(1.0f, 2.0f, 3.0f) + 1.0f.xxx;
    float3 _215 = float3(4.0f, 5.0f, 6.0f) + 1.0f.xxx;
    float3 _216 = float3(7.0f, 8.0f, 9.0f) + 1.0f.xxx;
    m3x3 = float3x3(_214, _215, _216);
    bool _234 = false;
    if (_195)
    {
        _234 = (all(bool3(_214.x == float3(2.0f, 3.0f, 4.0f).x, _214.y == float3(2.0f, 3.0f, 4.0f).y, _214.z == float3(2.0f, 3.0f, 4.0f).z)) && all(bool3(_215.x == float3(5.0f, 6.0f, 7.0f).x, _215.y == float3(5.0f, 6.0f, 7.0f).y, _215.z == float3(5.0f, 6.0f, 7.0f).z))) && all(bool3(_216.x == float3(8.0f, 9.0f, 10.0f).x, _216.y == float3(8.0f, 9.0f, 10.0f).y, _216.z == float3(8.0f, 9.0f, 10.0f).z));
    }
    else
    {
        _234 = false;
    }
    ok = _234;
    bool _254 = false;
    if (_234)
    {
        float3 _237 = _214 + 1.0f.xxx;
        float3 _238 = _215 + 1.0f.xxx;
        float3 _239 = _216 + 1.0f.xxx;
        m3x3 = float3x3(_237, _238, _239);
        _254 = (all(bool3(_237.x == float3(3.0f, 4.0f, 5.0f).x, _237.y == float3(3.0f, 4.0f, 5.0f).y, _237.z == float3(3.0f, 4.0f, 5.0f).z)) && all(bool3(_238.x == float3(6.0f, 7.0f, 8.0f).x, _238.y == float3(6.0f, 7.0f, 8.0f).y, _238.z == float3(6.0f, 7.0f, 8.0f).z))) && all(bool3(_239.x == float3(9.0f, 10.0f, 11.0f).x, _239.y == float3(9.0f, 10.0f, 11.0f).y, _239.z == float3(9.0f, 10.0f, 11.0f).z));
    }
    else
    {
        _254 = false;
    }
    ok = _254;
    bool _273 = false;
    if (_254)
    {
        float3x3 _257 = m3x3;
        float3 _259 = _257[0] - 1.0f.xxx;
        float3 _261 = _257[1] - 1.0f.xxx;
        float3 _263 = _257[2] - 1.0f.xxx;
        m3x3 = float3x3(_259, _261, _263);
        _273 = (all(bool3(_259.x == float3(2.0f, 3.0f, 4.0f).x, _259.y == float3(2.0f, 3.0f, 4.0f).y, _259.z == float3(2.0f, 3.0f, 4.0f).z)) && all(bool3(_261.x == float3(5.0f, 6.0f, 7.0f).x, _261.y == float3(5.0f, 6.0f, 7.0f).y, _261.z == float3(5.0f, 6.0f, 7.0f).z))) && all(bool3(_263.x == float3(8.0f, 9.0f, 10.0f).x, _263.y == float3(8.0f, 9.0f, 10.0f).y, _263.z == float3(8.0f, 9.0f, 10.0f).z));
    }
    else
    {
        _273 = false;
    }
    ok = _273;
    float3x3 _274 = m3x3;
    float3 _276 = _274[0] - 1.0f.xxx;
    float3 _278 = _274[1] - 1.0f.xxx;
    float3 _280 = _274[2] - 1.0f.xxx;
    m3x3 = float3x3(_276, _278, _280);
    bool _292 = false;
    if (_273)
    {
        _292 = (all(bool3(_276.x == float3(1.0f, 2.0f, 3.0f).x, _276.y == float3(1.0f, 2.0f, 3.0f).y, _276.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_278.x == float3(4.0f, 5.0f, 6.0f).x, _278.y == float3(4.0f, 5.0f, 6.0f).y, _278.z == float3(4.0f, 5.0f, 6.0f).z))) && all(bool3(_280.x == float3(7.0f, 8.0f, 9.0f).x, _280.y == float3(7.0f, 8.0f, 9.0f).y, _280.z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _292 = false;
    }
    ok = _292;
    bool _300 = false;
    if (_292)
    {
        _300 = _11_colorGreen.x != 1.0f;
    }
    else
    {
        _300 = false;
    }
    ok = _300;
    bool _309 = false;
    if (_300)
    {
        _309 = (-1.0f) == (-_11_colorGreen.y);
    }
    else
    {
        _309 = false;
    }
    ok = _309;
    bool _318 = false;
    if (_309)
    {
        float4 _315 = -_11_colorGreen;
        _318 = all(bool4(float4(0.0f, -1.0f, 0.0f, -1.0f).x == _315.x, float4(0.0f, -1.0f, 0.0f, -1.0f).y == _315.y, float4(0.0f, -1.0f, 0.0f, -1.0f).z == _315.z, float4(0.0f, -1.0f, 0.0f, -1.0f).w == _315.w));
    }
    else
    {
        _318 = false;
    }
    ok = _318;
    bool _341 = false;
    if (_318)
    {
        float2 _332 = -_11_testMatrix2x2[0];
        float2 _334 = -_11_testMatrix2x2[1];
        _341 = all(bool2(float2(-1.0f, -2.0f).x == _332.x, float2(-1.0f, -2.0f).y == _332.y)) && all(bool2(float2(-3.0f, -4.0f).x == _334.x, float2(-3.0f, -4.0f).y == _334.y));
    }
    else
    {
        _341 = false;
    }
    ok = _341;
    int2 _348 = int2(i, -i);
    int2 iv = _348;
    bool _355 = false;
    if (_341)
    {
        _355 = (-i) == (-5);
    }
    else
    {
        _355 = false;
    }
    ok = _355;
    bool _362 = false;
    if (_355)
    {
        int2 _358 = -_348;
        _362 = all(bool2(_358.x == int2(-5, 5).x, _358.y == int2(-5, 5).y));
    }
    else
    {
        _362 = false;
    }
    ok = _362;
    float4 _363 = 0.0f.xxxx;
    if (_362)
    {
        _363 = _11_colorGreen;
    }
    else
    {
        _363 = _11_colorRed;
    }
    return _363;
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
