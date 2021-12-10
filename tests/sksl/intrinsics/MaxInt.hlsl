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
    float4 _49 = _10_colorGreen * 100.0f;
    int4 intGreen = int4(int(_49.x), int(_49.y), int(_49.z), int(_49.w));
    int4 expectedA = int4(50, 50, 75, 225);
    int4 expectedB = int4(0, 100, 75, 225);
    bool _86 = false;
    if (max(intValues.x, 50) == expectedA.x)
    {
        int2 _76 = max(intValues.xy, int2(50, 50));
        _86 = all(bool2(_76.x == expectedA.xy.x, _76.y == expectedA.xy.y));
    }
    else
    {
        _86 = false;
    }
    bool _99 = false;
    if (_86)
    {
        int3 _89 = max(intValues.xyz, int3(50, 50, 50));
        _99 = all(bool3(_89.x == expectedA.xyz.x, _89.y == expectedA.xyz.y, _89.z == expectedA.xyz.z));
    }
    else
    {
        _99 = false;
    }
    bool _109 = false;
    if (_99)
    {
        int4 _102 = max(intValues, int4(50, 50, 50, 50));
        _109 = all(bool4(_102.x == expectedA.x, _102.y == expectedA.y, _102.z == expectedA.z, _102.w == expectedA.w));
    }
    else
    {
        _109 = false;
    }
    bool _115 = false;
    if (_109)
    {
        _115 = 50 == expectedA.x;
    }
    else
    {
        _115 = false;
    }
    bool _123 = false;
    if (_115)
    {
        _123 = all(bool2(int2(50, 50).x == expectedA.xy.x, int2(50, 50).y == expectedA.xy.y));
    }
    else
    {
        _123 = false;
    }
    bool _131 = false;
    if (_123)
    {
        _131 = all(bool3(int3(50, 50, 75).x == expectedA.xyz.x, int3(50, 50, 75).y == expectedA.xyz.y, int3(50, 50, 75).z == expectedA.xyz.z));
    }
    else
    {
        _131 = false;
    }
    bool _137 = false;
    if (_131)
    {
        _137 = all(bool4(int4(50, 50, 75, 225).x == expectedA.x, int4(50, 50, 75, 225).y == expectedA.y, int4(50, 50, 75, 225).z == expectedA.z, int4(50, 50, 75, 225).w == expectedA.w));
    }
    else
    {
        _137 = false;
    }
    bool _148 = false;
    if (_137)
    {
        _148 = max(intValues.x, intGreen.x) == expectedB.x;
    }
    else
    {
        _148 = false;
    }
    bool _160 = false;
    if (_148)
    {
        int2 _151 = max(intValues.xy, intGreen.xy);
        _160 = all(bool2(_151.x == expectedB.xy.x, _151.y == expectedB.xy.y));
    }
    else
    {
        _160 = false;
    }
    bool _172 = false;
    if (_160)
    {
        int3 _163 = max(intValues.xyz, intGreen.xyz);
        _172 = all(bool3(_163.x == expectedB.xyz.x, _163.y == expectedB.xyz.y, _163.z == expectedB.xyz.z));
    }
    else
    {
        _172 = false;
    }
    bool _181 = false;
    if (_172)
    {
        int4 _175 = max(intValues, intGreen);
        _181 = all(bool4(_175.x == expectedB.x, _175.y == expectedB.y, _175.z == expectedB.z, _175.w == expectedB.w));
    }
    else
    {
        _181 = false;
    }
    bool _187 = false;
    if (_181)
    {
        _187 = 0 == expectedB.x;
    }
    else
    {
        _187 = false;
    }
    bool _195 = false;
    if (_187)
    {
        _195 = all(bool2(int2(0, 100).x == expectedB.xy.x, int2(0, 100).y == expectedB.xy.y));
    }
    else
    {
        _195 = false;
    }
    bool _203 = false;
    if (_195)
    {
        _203 = all(bool3(int3(0, 100, 75).x == expectedB.xyz.x, int3(0, 100, 75).y == expectedB.xyz.y, int3(0, 100, 75).z == expectedB.xyz.z));
    }
    else
    {
        _203 = false;
    }
    bool _209 = false;
    if (_203)
    {
        _209 = all(bool4(int4(0, 100, 75, 225).x == expectedB.x, int4(0, 100, 75, 225).y == expectedB.y, int4(0, 100, 75, 225).z == expectedB.z, int4(0, 100, 75, 225).w == expectedB.w));
    }
    else
    {
        _209 = false;
    }
    float4 _210 = 0.0f.xxxx;
    if (_209)
    {
        _210 = _10_colorGreen;
    }
    else
    {
        _210 = _10_colorRed;
    }
    return _210;
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
