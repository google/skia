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
    float4 constVal = float4(-1.25f, 0.0f, 0.75f, 2.25f);
    float4 expectedA = float4(0.0f, 0.0f, 0.84375f, 1.0f);
    float4 expectedB = float4(1.0f, 0.0f, 1.0f, 1.0f);
    bool _49 = false;
    if (0.0f == expectedA.x)
    {
        _49 = all(bool2(0.0f.xx.x == expectedA.xy.x, 0.0f.xx.y == expectedA.xy.y));
    }
    else
    {
        _49 = false;
    }
    bool _59 = false;
    if (_49)
    {
        _59 = all(bool3(float3(0.0f, 0.0f, 0.84375f).x == expectedA.xyz.x, float3(0.0f, 0.0f, 0.84375f).y == expectedA.xyz.y, float3(0.0f, 0.0f, 0.84375f).z == expectedA.xyz.z));
    }
    else
    {
        _59 = false;
    }
    bool _66 = false;
    if (_59)
    {
        _66 = all(bool4(float4(0.0f, 0.0f, 0.84375f, 1.0f).x == expectedA.x, float4(0.0f, 0.0f, 0.84375f, 1.0f).y == expectedA.y, float4(0.0f, 0.0f, 0.84375f, 1.0f).z == expectedA.z, float4(0.0f, 0.0f, 0.84375f, 1.0f).w == expectedA.w));
    }
    else
    {
        _66 = false;
    }
    bool _72 = false;
    if (_66)
    {
        _72 = 0.0f == expectedA.x;
    }
    else
    {
        _72 = false;
    }
    bool _79 = false;
    if (_72)
    {
        _79 = all(bool2(0.0f.xx.x == expectedA.xy.x, 0.0f.xx.y == expectedA.xy.y));
    }
    else
    {
        _79 = false;
    }
    bool _86 = false;
    if (_79)
    {
        _86 = all(bool3(float3(0.0f, 0.0f, 0.84375f).x == expectedA.xyz.x, float3(0.0f, 0.0f, 0.84375f).y == expectedA.xyz.y, float3(0.0f, 0.0f, 0.84375f).z == expectedA.xyz.z));
    }
    else
    {
        _86 = false;
    }
    bool _92 = false;
    if (_86)
    {
        _92 = all(bool4(float4(0.0f, 0.0f, 0.84375f, 1.0f).x == expectedA.x, float4(0.0f, 0.0f, 0.84375f, 1.0f).y == expectedA.y, float4(0.0f, 0.0f, 0.84375f, 1.0f).z == expectedA.z, float4(0.0f, 0.0f, 0.84375f, 1.0f).w == expectedA.w));
    }
    else
    {
        _92 = false;
    }
    bool _109 = false;
    if (_92)
    {
        _109 = smoothstep(_10_colorRed.y, _10_colorGreen.y, -1.25f) == expectedA.x;
    }
    else
    {
        _109 = false;
    }
    bool _126 = false;
    if (_109)
    {
        float2 _112 = smoothstep(_10_colorRed.y.xx, _10_colorGreen.y.xx, float2(-1.25f, 0.0f));
        _126 = all(bool2(_112.x == expectedA.xy.x, _112.y == expectedA.xy.y));
    }
    else
    {
        _126 = false;
    }
    bool _143 = false;
    if (_126)
    {
        float3 _129 = smoothstep(_10_colorRed.y.xxx, _10_colorGreen.y.xxx, float3(-1.25f, 0.0f, 0.75f));
        _143 = all(bool3(_129.x == expectedA.xyz.x, _129.y == expectedA.xyz.y, _129.z == expectedA.xyz.z));
    }
    else
    {
        _143 = false;
    }
    bool _159 = false;
    if (_143)
    {
        float4 _146 = smoothstep(_10_colorRed.y.xxxx, _10_colorGreen.y.xxxx, constVal);
        _159 = all(bool4(_146.x == expectedA.x, _146.y == expectedA.y, _146.z == expectedA.z, _146.w == expectedA.w));
    }
    else
    {
        _159 = false;
    }
    bool _165 = false;
    if (_159)
    {
        _165 = 1.0f == expectedB.x;
    }
    else
    {
        _165 = false;
    }
    bool _173 = false;
    if (_165)
    {
        _173 = all(bool2(float2(1.0f, 0.0f).x == expectedB.xy.x, float2(1.0f, 0.0f).y == expectedB.xy.y));
    }
    else
    {
        _173 = false;
    }
    bool _181 = false;
    if (_173)
    {
        _181 = all(bool3(float3(1.0f, 0.0f, 1.0f).x == expectedB.xyz.x, float3(1.0f, 0.0f, 1.0f).y == expectedB.xyz.y, float3(1.0f, 0.0f, 1.0f).z == expectedB.xyz.z));
    }
    else
    {
        _181 = false;
    }
    bool _187 = false;
    if (_181)
    {
        _187 = all(bool4(float4(1.0f, 0.0f, 1.0f, 1.0f).x == expectedB.x, float4(1.0f, 0.0f, 1.0f, 1.0f).y == expectedB.y, float4(1.0f, 0.0f, 1.0f, 1.0f).z == expectedB.z, float4(1.0f, 0.0f, 1.0f, 1.0f).w == expectedB.w));
    }
    else
    {
        _187 = false;
    }
    bool _200 = false;
    if (_187)
    {
        _200 = smoothstep(_10_colorRed.x, _10_colorGreen.x, -1.25f) == expectedB.x;
    }
    else
    {
        _200 = false;
    }
    bool _214 = false;
    if (_200)
    {
        float2 _203 = smoothstep(_10_colorRed.xy, _10_colorGreen.xy, float2(-1.25f, 0.0f));
        _214 = all(bool2(_203.x == expectedB.xy.x, _203.y == expectedB.xy.y));
    }
    else
    {
        _214 = false;
    }
    bool _228 = false;
    if (_214)
    {
        float3 _217 = smoothstep(_10_colorRed.xyz, _10_colorGreen.xyz, float3(-1.25f, 0.0f, 0.75f));
        _228 = all(bool3(_217.x == expectedB.xyz.x, _217.y == expectedB.xyz.y, _217.z == expectedB.xyz.z));
    }
    else
    {
        _228 = false;
    }
    bool _240 = false;
    if (_228)
    {
        float4 _231 = smoothstep(_10_colorRed, _10_colorGreen, constVal);
        _240 = all(bool4(_231.x == expectedB.x, _231.y == expectedB.y, _231.z == expectedB.z, _231.w == expectedB.w));
    }
    else
    {
        _240 = false;
    }
    float4 _241 = 0.0f.xxxx;
    if (_240)
    {
        _241 = _10_colorGreen;
    }
    else
    {
        _241 = _10_colorRed;
    }
    return _241;
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
