cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 _35 = _10_testInputs * 100.0f;
    int4 intValues = int4(int(_35.x), int(_35.y), int(_35.z), int(_35.w));
    int4 expectedA = int4(-100, 0, 75, 100);
    int4 clampLow = int4(-100, -200, -200, 100);
    int4 expectedB = int4(-100, 0, 50, 225);
    int4 clampHigh = int4(100, 200, 50, 300);
    bool _81 = false;
    if (clamp(intValues.x, -100, 100) == expectedA.x)
    {
        int2 _70 = clamp(intValues.xy, int2(-100, -100), int2(100, 100));
        _81 = all(bool2(_70.x == expectedA.xy.x, _70.y == expectedA.xy.y));
    }
    else
    {
        _81 = false;
    }
    bool _95 = false;
    if (_81)
    {
        int3 _84 = clamp(intValues.xyz, int3(-100, -100, -100), int3(100, 100, 100));
        _95 = all(bool3(_84.x == expectedA.xyz.x, _84.y == expectedA.xyz.y, _84.z == expectedA.xyz.z));
    }
    else
    {
        _95 = false;
    }
    bool _106 = false;
    if (_95)
    {
        int4 _98 = clamp(intValues, int4(-100, -100, -100, -100), int4(100, 100, 100, 100));
        _106 = all(bool4(_98.x == expectedA.x, _98.y == expectedA.y, _98.z == expectedA.z, _98.w == expectedA.w));
    }
    else
    {
        _106 = false;
    }
    bool _112 = false;
    if (_106)
    {
        _112 = (-100) == expectedA.x;
    }
    else
    {
        _112 = false;
    }
    bool _120 = false;
    if (_112)
    {
        _120 = all(bool2(int2(-100, 0).x == expectedA.xy.x, int2(-100, 0).y == expectedA.xy.y));
    }
    else
    {
        _120 = false;
    }
    bool _128 = false;
    if (_120)
    {
        _128 = all(bool3(int3(-100, 0, 75).x == expectedA.xyz.x, int3(-100, 0, 75).y == expectedA.xyz.y, int3(-100, 0, 75).z == expectedA.xyz.z));
    }
    else
    {
        _128 = false;
    }
    bool _134 = false;
    if (_128)
    {
        _134 = all(bool4(int4(-100, 0, 75, 100).x == expectedA.x, int4(-100, 0, 75, 100).y == expectedA.y, int4(-100, 0, 75, 100).z == expectedA.z, int4(-100, 0, 75, 100).w == expectedA.w));
    }
    else
    {
        _134 = false;
    }
    bool _143 = false;
    if (_134)
    {
        _143 = clamp(intValues.x, -100, 100) == expectedB.x;
    }
    else
    {
        _143 = false;
    }
    bool _155 = false;
    if (_143)
    {
        int2 _146 = clamp(intValues.xy, int2(-100, -200), int2(100, 200));
        _155 = all(bool2(_146.x == expectedB.xy.x, _146.y == expectedB.xy.y));
    }
    else
    {
        _155 = false;
    }
    bool _167 = false;
    if (_155)
    {
        int3 _158 = clamp(intValues.xyz, int3(-100, -200, -200), int3(100, 200, 50));
        _167 = all(bool3(_158.x == expectedB.xyz.x, _158.y == expectedB.xyz.y, _158.z == expectedB.xyz.z));
    }
    else
    {
        _167 = false;
    }
    bool _177 = false;
    if (_167)
    {
        int4 _170 = clamp(intValues, clampLow, clampHigh);
        _177 = all(bool4(_170.x == expectedB.x, _170.y == expectedB.y, _170.z == expectedB.z, _170.w == expectedB.w));
    }
    else
    {
        _177 = false;
    }
    bool _183 = false;
    if (_177)
    {
        _183 = (-100) == expectedB.x;
    }
    else
    {
        _183 = false;
    }
    bool _190 = false;
    if (_183)
    {
        _190 = all(bool2(int2(-100, 0).x == expectedB.xy.x, int2(-100, 0).y == expectedB.xy.y));
    }
    else
    {
        _190 = false;
    }
    bool _198 = false;
    if (_190)
    {
        _198 = all(bool3(int3(-100, 0, 50).x == expectedB.xyz.x, int3(-100, 0, 50).y == expectedB.xyz.y, int3(-100, 0, 50).z == expectedB.xyz.z));
    }
    else
    {
        _198 = false;
    }
    bool _204 = false;
    if (_198)
    {
        _204 = all(bool4(int4(-100, 0, 50, 225).x == expectedB.x, int4(-100, 0, 50, 225).y == expectedB.y, int4(-100, 0, 50, 225).z == expectedB.z, int4(-100, 0, 50, 225).w == expectedB.w));
    }
    else
    {
        _204 = false;
    }
    float4 _205 = 0.0f.xxxx;
    if (_204)
    {
        _205 = _10_colorGreen;
    }
    else
    {
        _205 = _10_colorRed;
    }
    return _205;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
