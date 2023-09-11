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
    int4 expectedA = int4(-100, 0, 75, 100);
    int4 expectedB = int4(-100, 0, 50, 225);
    int _54 = _41.x;
    bool _67 = false;
    if (clamp(_54, -100, 100) == (-100))
    {
        int2 _58 = clamp(_41.xy, int2(-100, -100), int2(100, 100));
        _67 = all(bool2(_58.x == int4(-100, 0, 75, 100).xy.x, _58.y == int4(-100, 0, 75, 100).xy.y));
    }
    else
    {
        _67 = false;
    }
    bool _79 = false;
    if (_67)
    {
        int3 _70 = clamp(_41.xyz, int3(-100, -100, -100), int3(100, 100, 100));
        _79 = all(bool3(_70.x == int4(-100, 0, 75, 100).xyz.x, _70.y == int4(-100, 0, 75, 100).xyz.y, _70.z == int4(-100, 0, 75, 100).xyz.z));
    }
    else
    {
        _79 = false;
    }
    bool _88 = false;
    if (_79)
    {
        int4 _82 = clamp(_41, int4(-100, -100, -100, -100), int4(100, 100, 100, 100));
        _88 = all(bool4(_82.x == int4(-100, 0, 75, 100).x, _82.y == int4(-100, 0, 75, 100).y, _82.z == int4(-100, 0, 75, 100).z, _82.w == int4(-100, 0, 75, 100).w));
    }
    else
    {
        _88 = false;
    }
    bool _92 = false;
    if (_88)
    {
        _92 = true;
    }
    else
    {
        _92 = false;
    }
    bool _99 = false;
    if (_92)
    {
        _99 = all(bool2(int2(-100, 0).x == int4(-100, 0, 75, 100).xy.x, int2(-100, 0).y == int4(-100, 0, 75, 100).xy.y));
    }
    else
    {
        _99 = false;
    }
    bool _106 = false;
    if (_99)
    {
        _106 = all(bool3(int3(-100, 0, 75).x == int4(-100, 0, 75, 100).xyz.x, int3(-100, 0, 75).y == int4(-100, 0, 75, 100).xyz.y, int3(-100, 0, 75).z == int4(-100, 0, 75, 100).xyz.z));
    }
    else
    {
        _106 = false;
    }
    bool _109 = false;
    if (_106)
    {
        _109 = true;
    }
    else
    {
        _109 = false;
    }
    bool _114 = false;
    if (_109)
    {
        _114 = clamp(_54, -100, 100) == (-100);
    }
    else
    {
        _114 = false;
    }
    bool _126 = false;
    if (_114)
    {
        int2 _117 = clamp(_41.xy, int2(-100, -200), int2(100, 200));
        _126 = all(bool2(_117.x == int4(-100, 0, 50, 225).xy.x, _117.y == int4(-100, 0, 50, 225).xy.y));
    }
    else
    {
        _126 = false;
    }
    bool _136 = false;
    if (_126)
    {
        int3 _129 = clamp(_41.xyz, int3(-100, -200, -200), int3(100, 200, 50));
        _136 = all(bool3(_129.x == int4(-100, 0, 50, 225).xyz.x, _129.y == int4(-100, 0, 50, 225).xyz.y, _129.z == int4(-100, 0, 50, 225).xyz.z));
    }
    else
    {
        _136 = false;
    }
    bool _145 = false;
    if (_136)
    {
        int4 _139 = clamp(_41, int4(-100, -200, -200, 100), int4(100, 200, 50, 300));
        _145 = all(bool4(_139.x == int4(-100, 0, 50, 225).x, _139.y == int4(-100, 0, 50, 225).y, _139.z == int4(-100, 0, 50, 225).z, _139.w == int4(-100, 0, 50, 225).w));
    }
    else
    {
        _145 = false;
    }
    bool _148 = false;
    if (_145)
    {
        _148 = true;
    }
    else
    {
        _148 = false;
    }
    bool _154 = false;
    if (_148)
    {
        _154 = all(bool2(int2(-100, 0).x == int4(-100, 0, 50, 225).xy.x, int2(-100, 0).y == int4(-100, 0, 50, 225).xy.y));
    }
    else
    {
        _154 = false;
    }
    bool _161 = false;
    if (_154)
    {
        _161 = all(bool3(int3(-100, 0, 50).x == int4(-100, 0, 50, 225).xyz.x, int3(-100, 0, 50).y == int4(-100, 0, 50, 225).xyz.y, int3(-100, 0, 50).z == int4(-100, 0, 50, 225).xyz.z));
    }
    else
    {
        _161 = false;
    }
    bool _164 = false;
    if (_161)
    {
        _164 = true;
    }
    else
    {
        _164 = false;
    }
    float4 _165 = 0.0f.xxxx;
    if (_164)
    {
        _165 = _7_colorGreen;
    }
    else
    {
        _165 = _7_colorRed;
    }
    return _165;
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
