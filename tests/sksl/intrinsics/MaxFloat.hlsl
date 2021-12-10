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
    float4 expectedA = float4(0.5f, 0.5f, 0.75f, 2.25f);
    float4 expectedB = float4(0.0f, 1.0f, 0.75f, 2.25f);
    bool _58 = false;
    if (max(_10_testInputs.x, 0.5f) == expectedA.x)
    {
        float2 _48 = max(_10_testInputs.xy, 0.5f.xx);
        _58 = all(bool2(_48.x == expectedA.xy.x, _48.y == expectedA.xy.y));
    }
    else
    {
        _58 = false;
    }
    bool _72 = false;
    if (_58)
    {
        float3 _61 = max(_10_testInputs.xyz, 0.5f.xxx);
        _72 = all(bool3(_61.x == expectedA.xyz.x, _61.y == expectedA.xyz.y, _61.z == expectedA.xyz.z));
    }
    else
    {
        _72 = false;
    }
    bool _83 = false;
    if (_72)
    {
        float4 _75 = max(_10_testInputs, 0.5f.xxxx);
        _83 = all(bool4(_75.x == expectedA.x, _75.y == expectedA.y, _75.z == expectedA.z, _75.w == expectedA.w));
    }
    else
    {
        _83 = false;
    }
    bool _89 = false;
    if (_83)
    {
        _89 = 0.5f == expectedA.x;
    }
    else
    {
        _89 = false;
    }
    bool _97 = false;
    if (_89)
    {
        _97 = all(bool2(0.5f.xx.x == expectedA.xy.x, 0.5f.xx.y == expectedA.xy.y));
    }
    else
    {
        _97 = false;
    }
    bool _105 = false;
    if (_97)
    {
        _105 = all(bool3(float3(0.5f, 0.5f, 0.75f).x == expectedA.xyz.x, float3(0.5f, 0.5f, 0.75f).y == expectedA.xyz.y, float3(0.5f, 0.5f, 0.75f).z == expectedA.xyz.z));
    }
    else
    {
        _105 = false;
    }
    bool _111 = false;
    if (_105)
    {
        _111 = all(bool4(float4(0.5f, 0.5f, 0.75f, 2.25f).x == expectedA.x, float4(0.5f, 0.5f, 0.75f, 2.25f).y == expectedA.y, float4(0.5f, 0.5f, 0.75f, 2.25f).z == expectedA.z, float4(0.5f, 0.5f, 0.75f, 2.25f).w == expectedA.w));
    }
    else
    {
        _111 = false;
    }
    bool _125 = false;
    if (_111)
    {
        _125 = max(_10_testInputs.x, _10_colorGreen.x) == expectedB.x;
    }
    else
    {
        _125 = false;
    }
    bool _139 = false;
    if (_125)
    {
        float2 _128 = max(_10_testInputs.xy, _10_colorGreen.xy);
        _139 = all(bool2(_128.x == expectedB.xy.x, _128.y == expectedB.xy.y));
    }
    else
    {
        _139 = false;
    }
    bool _153 = false;
    if (_139)
    {
        float3 _142 = max(_10_testInputs.xyz, _10_colorGreen.xyz);
        _153 = all(bool3(_142.x == expectedB.xyz.x, _142.y == expectedB.xyz.y, _142.z == expectedB.xyz.z));
    }
    else
    {
        _153 = false;
    }
    bool _164 = false;
    if (_153)
    {
        float4 _156 = max(_10_testInputs, _10_colorGreen);
        _164 = all(bool4(_156.x == expectedB.x, _156.y == expectedB.y, _156.z == expectedB.z, _156.w == expectedB.w));
    }
    else
    {
        _164 = false;
    }
    bool _170 = false;
    if (_164)
    {
        _170 = 0.0f == expectedB.x;
    }
    else
    {
        _170 = false;
    }
    bool _178 = false;
    if (_170)
    {
        _178 = all(bool2(float2(0.0f, 1.0f).x == expectedB.xy.x, float2(0.0f, 1.0f).y == expectedB.xy.y));
    }
    else
    {
        _178 = false;
    }
    bool _186 = false;
    if (_178)
    {
        _186 = all(bool3(float3(0.0f, 1.0f, 0.75f).x == expectedB.xyz.x, float3(0.0f, 1.0f, 0.75f).y == expectedB.xyz.y, float3(0.0f, 1.0f, 0.75f).z == expectedB.xyz.z));
    }
    else
    {
        _186 = false;
    }
    bool _192 = false;
    if (_186)
    {
        _192 = all(bool4(float4(0.0f, 1.0f, 0.75f, 2.25f).x == expectedB.x, float4(0.0f, 1.0f, 0.75f, 2.25f).y == expectedB.y, float4(0.0f, 1.0f, 0.75f, 2.25f).z == expectedB.z, float4(0.0f, 1.0f, 0.75f, 2.25f).w == expectedB.w));
    }
    else
    {
        _192 = false;
    }
    float4 _193 = 0.0f.xxxx;
    if (_192)
    {
        _193 = _10_colorGreen;
    }
    else
    {
        _193 = _10_colorRed;
    }
    return _193;
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
