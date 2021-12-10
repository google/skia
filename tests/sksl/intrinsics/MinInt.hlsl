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
    int4 expectedA = int4(-125, 0, 50, 50);
    int4 expectedB = int4(-125, 0, 0, 100);
    bool _85 = false;
    if (min(intValues.x, 50) == expectedA.x)
    {
        int2 _75 = min(intValues.xy, int2(50, 50));
        _85 = all(bool2(_75.x == expectedA.xy.x, _75.y == expectedA.xy.y));
    }
    else
    {
        _85 = false;
    }
    bool _98 = false;
    if (_85)
    {
        int3 _88 = min(intValues.xyz, int3(50, 50, 50));
        _98 = all(bool3(_88.x == expectedA.xyz.x, _88.y == expectedA.xyz.y, _88.z == expectedA.xyz.z));
    }
    else
    {
        _98 = false;
    }
    bool _108 = false;
    if (_98)
    {
        int4 _101 = min(intValues, int4(50, 50, 50, 50));
        _108 = all(bool4(_101.x == expectedA.x, _101.y == expectedA.y, _101.z == expectedA.z, _101.w == expectedA.w));
    }
    else
    {
        _108 = false;
    }
    bool _114 = false;
    if (_108)
    {
        _114 = (-125) == expectedA.x;
    }
    else
    {
        _114 = false;
    }
    bool _122 = false;
    if (_114)
    {
        _122 = all(bool2(int2(-125, 0).x == expectedA.xy.x, int2(-125, 0).y == expectedA.xy.y));
    }
    else
    {
        _122 = false;
    }
    bool _130 = false;
    if (_122)
    {
        _130 = all(bool3(int3(-125, 0, 50).x == expectedA.xyz.x, int3(-125, 0, 50).y == expectedA.xyz.y, int3(-125, 0, 50).z == expectedA.xyz.z));
    }
    else
    {
        _130 = false;
    }
    bool _136 = false;
    if (_130)
    {
        _136 = all(bool4(int4(-125, 0, 50, 50).x == expectedA.x, int4(-125, 0, 50, 50).y == expectedA.y, int4(-125, 0, 50, 50).z == expectedA.z, int4(-125, 0, 50, 50).w == expectedA.w));
    }
    else
    {
        _136 = false;
    }
    bool _147 = false;
    if (_136)
    {
        _147 = min(intValues.x, intGreen.x) == expectedB.x;
    }
    else
    {
        _147 = false;
    }
    bool _159 = false;
    if (_147)
    {
        int2 _150 = min(intValues.xy, intGreen.xy);
        _159 = all(bool2(_150.x == expectedB.xy.x, _150.y == expectedB.xy.y));
    }
    else
    {
        _159 = false;
    }
    bool _171 = false;
    if (_159)
    {
        int3 _162 = min(intValues.xyz, intGreen.xyz);
        _171 = all(bool3(_162.x == expectedB.xyz.x, _162.y == expectedB.xyz.y, _162.z == expectedB.xyz.z));
    }
    else
    {
        _171 = false;
    }
    bool _180 = false;
    if (_171)
    {
        int4 _174 = min(intValues, intGreen);
        _180 = all(bool4(_174.x == expectedB.x, _174.y == expectedB.y, _174.z == expectedB.z, _174.w == expectedB.w));
    }
    else
    {
        _180 = false;
    }
    bool _186 = false;
    if (_180)
    {
        _186 = (-125) == expectedB.x;
    }
    else
    {
        _186 = false;
    }
    bool _193 = false;
    if (_186)
    {
        _193 = all(bool2(int2(-125, 0).x == expectedB.xy.x, int2(-125, 0).y == expectedB.xy.y));
    }
    else
    {
        _193 = false;
    }
    bool _201 = false;
    if (_193)
    {
        _201 = all(bool3(int3(-125, 0, 0).x == expectedB.xyz.x, int3(-125, 0, 0).y == expectedB.xyz.y, int3(-125, 0, 0).z == expectedB.xyz.z));
    }
    else
    {
        _201 = false;
    }
    bool _207 = false;
    if (_201)
    {
        _207 = all(bool4(int4(-125, 0, 0, 100).x == expectedB.x, int4(-125, 0, 0, 100).y == expectedB.y, int4(-125, 0, 0, 100).z == expectedB.z, int4(-125, 0, 0, 100).w == expectedB.w));
    }
    else
    {
        _207 = false;
    }
    float4 _208 = 0.0f.xxxx;
    if (_207)
    {
        _208 = _10_colorGreen;
    }
    else
    {
        _208 = _10_colorRed;
    }
    return _208;
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
