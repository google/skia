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
    float4 constGreen = float4(0.0f, 1.0f, 0.0f, 1.0f);
    float4 expectedA = float4(0.0f, 0.0f, 1.0f, 1.0f);
    float4 expectedB = float4(1.0f, 1.0f, 0.0f, 0.0f);
    bool _58 = false;
    if (step(0.5f, _10_testInputs.x) == expectedA.x)
    {
        float2 _48 = step(0.5f.xx, _10_testInputs.xy);
        _58 = all(bool2(_48.x == expectedA.xy.x, _48.y == expectedA.xy.y));
    }
    else
    {
        _58 = false;
    }
    bool _72 = false;
    if (_58)
    {
        float3 _61 = step(0.5f.xxx, _10_testInputs.xyz);
        _72 = all(bool3(_61.x == expectedA.xyz.x, _61.y == expectedA.xyz.y, _61.z == expectedA.xyz.z));
    }
    else
    {
        _72 = false;
    }
    bool _83 = false;
    if (_72)
    {
        float4 _75 = step(0.5f.xxxx, _10_testInputs);
        _83 = all(bool4(_75.x == expectedA.x, _75.y == expectedA.y, _75.z == expectedA.z, _75.w == expectedA.w));
    }
    else
    {
        _83 = false;
    }
    bool _89 = false;
    if (_83)
    {
        _89 = 0.0f == expectedA.x;
    }
    else
    {
        _89 = false;
    }
    bool _96 = false;
    if (_89)
    {
        _96 = all(bool2(0.0f.xx.x == expectedA.xy.x, 0.0f.xx.y == expectedA.xy.y));
    }
    else
    {
        _96 = false;
    }
    bool _104 = false;
    if (_96)
    {
        _104 = all(bool3(float3(0.0f, 0.0f, 1.0f).x == expectedA.xyz.x, float3(0.0f, 0.0f, 1.0f).y == expectedA.xyz.y, float3(0.0f, 0.0f, 1.0f).z == expectedA.xyz.z));
    }
    else
    {
        _104 = false;
    }
    bool _110 = false;
    if (_104)
    {
        _110 = all(bool4(float4(0.0f, 0.0f, 1.0f, 1.0f).x == expectedA.x, float4(0.0f, 0.0f, 1.0f, 1.0f).y == expectedA.y, float4(0.0f, 0.0f, 1.0f, 1.0f).z == expectedA.z, float4(0.0f, 0.0f, 1.0f, 1.0f).w == expectedA.w));
    }
    else
    {
        _110 = false;
    }
    bool _120 = false;
    if (_110)
    {
        _120 = step(_10_testInputs.x, 0.0f) == expectedB.x;
    }
    else
    {
        _120 = false;
    }
    bool _132 = false;
    if (_120)
    {
        float2 _123 = step(_10_testInputs.xy, float2(0.0f, 1.0f));
        _132 = all(bool2(_123.x == expectedB.xy.x, _123.y == expectedB.xy.y));
    }
    else
    {
        _132 = false;
    }
    bool _144 = false;
    if (_132)
    {
        float3 _135 = step(_10_testInputs.xyz, float3(0.0f, 1.0f, 0.0f));
        _144 = all(bool3(_135.x == expectedB.xyz.x, _135.y == expectedB.xyz.y, _135.z == expectedB.xyz.z));
    }
    else
    {
        _144 = false;
    }
    bool _154 = false;
    if (_144)
    {
        float4 _147 = step(_10_testInputs, constGreen);
        _154 = all(bool4(_147.x == expectedB.x, _147.y == expectedB.y, _147.z == expectedB.z, _147.w == expectedB.w));
    }
    else
    {
        _154 = false;
    }
    bool _160 = false;
    if (_154)
    {
        _160 = 1.0f == expectedB.x;
    }
    else
    {
        _160 = false;
    }
    bool _168 = false;
    if (_160)
    {
        _168 = all(bool2(1.0f.xx.x == expectedB.xy.x, 1.0f.xx.y == expectedB.xy.y));
    }
    else
    {
        _168 = false;
    }
    bool _176 = false;
    if (_168)
    {
        _176 = all(bool3(float3(1.0f, 1.0f, 0.0f).x == expectedB.xyz.x, float3(1.0f, 1.0f, 0.0f).y == expectedB.xyz.y, float3(1.0f, 1.0f, 0.0f).z == expectedB.xyz.z));
    }
    else
    {
        _176 = false;
    }
    bool _182 = false;
    if (_176)
    {
        _182 = all(bool4(float4(1.0f, 1.0f, 0.0f, 0.0f).x == expectedB.x, float4(1.0f, 1.0f, 0.0f, 0.0f).y == expectedB.y, float4(1.0f, 1.0f, 0.0f, 0.0f).z == expectedB.z, float4(1.0f, 1.0f, 0.0f, 0.0f).w == expectedB.w));
    }
    else
    {
        _182 = false;
    }
    float4 _183 = 0.0f.xxxx;
    if (_182)
    {
        _183 = _10_colorGreen;
    }
    else
    {
        _183 = _10_colorRed;
    }
    return _183;
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
