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
    float4 expectedA = float4(-1.0f, 0.0f, 0.0f, 2.0f);
    bool _50 = false;
    if (trunc(_10_testInputs.x) == (-1.0f))
    {
        float2 _42 = trunc(_10_testInputs.xy);
        _50 = all(bool2(_42.x == float2(-1.0f, 0.0f).x, _42.y == float2(-1.0f, 0.0f).y));
    }
    else
    {
        _50 = false;
    }
    bool _62 = false;
    if (_50)
    {
        float3 _53 = trunc(_10_testInputs.xyz);
        _62 = all(bool3(_53.x == float3(-1.0f, 0.0f, 0.0f).x, _53.y == float3(-1.0f, 0.0f, 0.0f).y, _53.z == float3(-1.0f, 0.0f, 0.0f).z));
    }
    else
    {
        _62 = false;
    }
    bool _72 = false;
    if (_62)
    {
        float4 _65 = trunc(_10_testInputs);
        _72 = all(bool4(_65.x == expectedA.x, _65.y == expectedA.y, _65.z == expectedA.z, _65.w == expectedA.w));
    }
    else
    {
        _72 = false;
    }
    float4 _73 = 0.0f.xxxx;
    if (_72)
    {
        _73 = _10_colorGreen;
    }
    else
    {
        _73 = _10_colorRed;
    }
    return _73;
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
