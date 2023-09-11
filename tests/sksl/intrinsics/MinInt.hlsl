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
    int4 expectedA = int4(-125, 0, 50, 50);
    int4 expectedB = int4(-125, 0, 0, 100);
    int _66 = _41.x;
    bool _78 = false;
    if (min(_66, 50) == (-125))
    {
        int2 _70 = min(_41.xy, int2(50, 50));
        _78 = all(bool2(_70.x == int4(-125, 0, 50, 50).xy.x, _70.y == int4(-125, 0, 50, 50).xy.y));
    }
    else
    {
        _78 = false;
    }
    bool _89 = false;
    if (_78)
    {
        int3 _81 = min(_41.xyz, int3(50, 50, 50));
        _89 = all(bool3(_81.x == int4(-125, 0, 50, 50).xyz.x, _81.y == int4(-125, 0, 50, 50).xyz.y, _81.z == int4(-125, 0, 50, 50).xyz.z));
    }
    else
    {
        _89 = false;
    }
    bool _97 = false;
    if (_89)
    {
        int4 _92 = min(_41, int4(50, 50, 50, 50));
        _97 = all(bool4(_92.x == int4(-125, 0, 50, 50).x, _92.y == int4(-125, 0, 50, 50).y, _92.z == int4(-125, 0, 50, 50).z, _92.w == int4(-125, 0, 50, 50).w));
    }
    else
    {
        _97 = false;
    }
    bool _101 = false;
    if (_97)
    {
        _101 = true;
    }
    else
    {
        _101 = false;
    }
    bool _108 = false;
    if (_101)
    {
        _108 = all(bool2(int2(-125, 0).x == int4(-125, 0, 50, 50).xy.x, int2(-125, 0).y == int4(-125, 0, 50, 50).xy.y));
    }
    else
    {
        _108 = false;
    }
    bool _115 = false;
    if (_108)
    {
        _115 = all(bool3(int3(-125, 0, 50).x == int4(-125, 0, 50, 50).xyz.x, int3(-125, 0, 50).y == int4(-125, 0, 50, 50).xyz.y, int3(-125, 0, 50).z == int4(-125, 0, 50, 50).xyz.z));
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
        _124 = min(_66, _55.x) == (-125);
    }
    else
    {
        _124 = false;
    }
    bool _133 = false;
    if (_124)
    {
        int2 _127 = min(_41.xy, _55.xy);
        _133 = all(bool2(_127.x == int4(-125, 0, 0, 100).xy.x, _127.y == int4(-125, 0, 0, 100).xy.y));
    }
    else
    {
        _133 = false;
    }
    bool _142 = false;
    if (_133)
    {
        int3 _136 = min(_41.xyz, _55.xyz);
        _142 = all(bool3(_136.x == int4(-125, 0, 0, 100).xyz.x, _136.y == int4(-125, 0, 0, 100).xyz.y, _136.z == int4(-125, 0, 0, 100).xyz.z));
    }
    else
    {
        _142 = false;
    }
    bool _148 = false;
    if (_142)
    {
        int4 _145 = min(_41, _55);
        _148 = all(bool4(_145.x == int4(-125, 0, 0, 100).x, _145.y == int4(-125, 0, 0, 100).y, _145.z == int4(-125, 0, 0, 100).z, _145.w == int4(-125, 0, 0, 100).w));
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
        _157 = all(bool2(int2(-125, 0).x == int4(-125, 0, 0, 100).xy.x, int2(-125, 0).y == int4(-125, 0, 0, 100).xy.y));
    }
    else
    {
        _157 = false;
    }
    bool _164 = false;
    if (_157)
    {
        _164 = all(bool3(int3(-125, 0, 0).x == int4(-125, 0, 0, 100).xyz.x, int3(-125, 0, 0).y == int4(-125, 0, 0, 100).xyz.y, int3(-125, 0, 0).z == int4(-125, 0, 0, 100).xyz.z));
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
        _168 = _7_colorGreen;
    }
    else
    {
        _168 = _7_colorRed;
    }
    return _168;
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
