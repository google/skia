cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 _35 = _11_testInputs * 100.0f;
    int4 _44 = int4(int(_35.x), int(_35.y), int(_35.z), int(_35.w));
    int4 intValues = _44;
    int4 expectedA = int4(-100, 0, 75, 100);
    int4 expectedB = int4(-100, 0, 50, 225);
    int _57 = _44.x;
    bool _70 = false;
    if (clamp(_57, -100, 100) == (-100))
    {
        int2 _61 = clamp(_44.xy, int2(-100, -100), int2(100, 100));
        _70 = all(bool2(_61.x == int4(-100, 0, 75, 100).xy.x, _61.y == int4(-100, 0, 75, 100).xy.y));
    }
    else
    {
        _70 = false;
    }
    bool _82 = false;
    if (_70)
    {
        int3 _73 = clamp(_44.xyz, int3(-100, -100, -100), int3(100, 100, 100));
        _82 = all(bool3(_73.x == int4(-100, 0, 75, 100).xyz.x, _73.y == int4(-100, 0, 75, 100).xyz.y, _73.z == int4(-100, 0, 75, 100).xyz.z));
    }
    else
    {
        _82 = false;
    }
    bool _91 = false;
    if (_82)
    {
        int4 _85 = clamp(_44, int4(-100, -100, -100, -100), int4(100, 100, 100, 100));
        _91 = all(bool4(_85.x == int4(-100, 0, 75, 100).x, _85.y == int4(-100, 0, 75, 100).y, _85.z == int4(-100, 0, 75, 100).z, _85.w == int4(-100, 0, 75, 100).w));
    }
    else
    {
        _91 = false;
    }
    bool _95 = false;
    if (_91)
    {
        _95 = true;
    }
    else
    {
        _95 = false;
    }
    bool _102 = false;
    if (_95)
    {
        _102 = all(bool2(int2(-100, 0).x == int4(-100, 0, 75, 100).xy.x, int2(-100, 0).y == int4(-100, 0, 75, 100).xy.y));
    }
    else
    {
        _102 = false;
    }
    bool _109 = false;
    if (_102)
    {
        _109 = all(bool3(int3(-100, 0, 75).x == int4(-100, 0, 75, 100).xyz.x, int3(-100, 0, 75).y == int4(-100, 0, 75, 100).xyz.y, int3(-100, 0, 75).z == int4(-100, 0, 75, 100).xyz.z));
    }
    else
    {
        _109 = false;
    }
    bool _112 = false;
    if (_109)
    {
        _112 = true;
    }
    else
    {
        _112 = false;
    }
    bool _117 = false;
    if (_112)
    {
        _117 = clamp(_57, -100, 100) == (-100);
    }
    else
    {
        _117 = false;
    }
    bool _129 = false;
    if (_117)
    {
        int2 _120 = clamp(_44.xy, int2(-100, -200), int2(100, 200));
        _129 = all(bool2(_120.x == int4(-100, 0, 50, 225).xy.x, _120.y == int4(-100, 0, 50, 225).xy.y));
    }
    else
    {
        _129 = false;
    }
    bool _139 = false;
    if (_129)
    {
        int3 _132 = clamp(_44.xyz, int3(-100, -200, -200), int3(100, 200, 50));
        _139 = all(bool3(_132.x == int4(-100, 0, 50, 225).xyz.x, _132.y == int4(-100, 0, 50, 225).xyz.y, _132.z == int4(-100, 0, 50, 225).xyz.z));
    }
    else
    {
        _139 = false;
    }
    bool _148 = false;
    if (_139)
    {
        int4 _142 = clamp(_44, int4(-100, -200, -200, 100), int4(100, 200, 50, 300));
        _148 = all(bool4(_142.x == int4(-100, 0, 50, 225).x, _142.y == int4(-100, 0, 50, 225).y, _142.z == int4(-100, 0, 50, 225).z, _142.w == int4(-100, 0, 50, 225).w));
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
    bool _157 = false;
    if (_151)
    {
        _157 = all(bool2(int2(-100, 0).x == int4(-100, 0, 50, 225).xy.x, int2(-100, 0).y == int4(-100, 0, 50, 225).xy.y));
    }
    else
    {
        _157 = false;
    }
    bool _164 = false;
    if (_157)
    {
        _164 = all(bool3(int3(-100, 0, 50).x == int4(-100, 0, 50, 225).xyz.x, int3(-100, 0, 50).y == int4(-100, 0, 50, 225).xyz.y, int3(-100, 0, 50).z == int4(-100, 0, 50, 225).xyz.z));
    }
    else
    {
        _164 = false;
    }
    bool _167 = false;
    if (_164)
    {
        _167 = true;
    }
    else
    {
        _167 = false;
    }
    float4 _168 = 0.0f.xxxx;
    if (_167)
    {
        _168 = _11_colorGreen;
    }
    else
    {
        _168 = _11_colorRed;
    }
    return _168;
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
