cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
    float4 _7_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 _32 = _7_testInputs * 100.0f;
    int4 _41 = int4(int(_32.x), int(_32.y), int(_32.z), int(_32.w));
    int4 intValues = _41;
    float4 _46 = _7_colorGreen * 100.0f;
    int4 _55 = int4(int(_46.x), int(_46.y), int(_46.z), int(_46.w));
    int4 intGreen = _55;
    int4 expectedA = int4(50, 50, 75, 225);
    int4 expectedB = int4(0, 100, 75, 225);
    int _67 = _41.x;
    bool _79 = false;
    if (max(_67, 50) == 50)
    {
        int2 _71 = max(_41.xy, int2(50, 50));
        _79 = all(bool2(_71.x == int4(50, 50, 75, 225).xy.x, _71.y == int4(50, 50, 75, 225).xy.y));
    }
    else
    {
        _79 = false;
    }
    bool _90 = false;
    if (_79)
    {
        int3 _82 = max(_41.xyz, int3(50, 50, 50));
        _90 = all(bool3(_82.x == int4(50, 50, 75, 225).xyz.x, _82.y == int4(50, 50, 75, 225).xyz.y, _82.z == int4(50, 50, 75, 225).xyz.z));
    }
    else
    {
        _90 = false;
    }
    bool _98 = false;
    if (_90)
    {
        int4 _93 = max(_41, int4(50, 50, 50, 50));
        _98 = all(bool4(_93.x == int4(50, 50, 75, 225).x, _93.y == int4(50, 50, 75, 225).y, _93.z == int4(50, 50, 75, 225).z, _93.w == int4(50, 50, 75, 225).w));
    }
    else
    {
        _98 = false;
    }
    bool _102 = false;
    if (_98)
    {
        _102 = true;
    }
    else
    {
        _102 = false;
    }
    bool _108 = false;
    if (_102)
    {
        _108 = all(bool2(int2(50, 50).x == int4(50, 50, 75, 225).xy.x, int2(50, 50).y == int4(50, 50, 75, 225).xy.y));
    }
    else
    {
        _108 = false;
    }
    bool _115 = false;
    if (_108)
    {
        _115 = all(bool3(int3(50, 50, 75).x == int4(50, 50, 75, 225).xyz.x, int3(50, 50, 75).y == int4(50, 50, 75, 225).xyz.y, int3(50, 50, 75).z == int4(50, 50, 75, 225).xyz.z));
    }
    else
    {
        _115 = false;
    }
    bool _118 = false;
    if (_115)
    {
        _118 = true;
    }
    else
    {
        _118 = false;
    }
    bool _124 = false;
    if (_118)
    {
        _124 = max(_67, _55.x) == 0;
    }
    else
    {
        _124 = false;
    }
    bool _133 = false;
    if (_124)
    {
        int2 _127 = max(_41.xy, _55.xy);
        _133 = all(bool2(_127.x == int4(0, 100, 75, 225).xy.x, _127.y == int4(0, 100, 75, 225).xy.y));
    }
    else
    {
        _133 = false;
    }
    bool _142 = false;
    if (_133)
    {
        int3 _136 = max(_41.xyz, _55.xyz);
        _142 = all(bool3(_136.x == int4(0, 100, 75, 225).xyz.x, _136.y == int4(0, 100, 75, 225).xyz.y, _136.z == int4(0, 100, 75, 225).xyz.z));
    }
    else
    {
        _142 = false;
    }
    bool _148 = false;
    if (_142)
    {
        int4 _145 = max(_41, _55);
        _148 = all(bool4(_145.x == int4(0, 100, 75, 225).x, _145.y == int4(0, 100, 75, 225).y, _145.z == int4(0, 100, 75, 225).z, _145.w == int4(0, 100, 75, 225).w));
    }
    else
    {
        _148 = false;
    }
    bool _151 = false;
    if (_148)
    {
        _151 = true;
    }
    else
    {
        _151 = false;
    }
    bool _158 = false;
    if (_151)
    {
        _158 = all(bool2(int2(0, 100).x == int4(0, 100, 75, 225).xy.x, int2(0, 100).y == int4(0, 100, 75, 225).xy.y));
    }
    else
    {
        _158 = false;
    }
    bool _165 = false;
    if (_158)
    {
        _165 = all(bool3(int3(0, 100, 75).x == int4(0, 100, 75, 225).xyz.x, int3(0, 100, 75).y == int4(0, 100, 75, 225).xyz.y, int3(0, 100, 75).z == int4(0, 100, 75, 225).xyz.z));
    }
    else
    {
        _165 = false;
    }
    bool _168 = false;
    if (_165)
    {
        _168 = true;
    }
    else
    {
        _168 = false;
    }
    float4 _169 = 0.0f.xxxx;
    if (_168)
    {
        _169 = _7_colorGreen;
    }
    else
    {
        _169 = _7_colorRed;
    }
    return _169;
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
