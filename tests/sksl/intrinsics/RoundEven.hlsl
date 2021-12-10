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
    float4 expectedA = float4(-1.0f, 0.0f, 1.0f, 2.0f);
    bool _51 = false;
    if (round(_10_testInputs.x) == (-1.0f))
    {
        float2 _43 = round(_10_testInputs.xy);
        _51 = all(bool2(_43.x == float2(-1.0f, 0.0f).x, _43.y == float2(-1.0f, 0.0f).y));
    }
    else
    {
        _51 = false;
    }
    bool _63 = false;
    if (_51)
    {
        float3 _54 = round(_10_testInputs.xyz);
        _63 = all(bool3(_54.x == float3(-1.0f, 0.0f, 1.0f).x, _54.y == float3(-1.0f, 0.0f, 1.0f).y, _54.z == float3(-1.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _63 = false;
    }
    bool _73 = false;
    if (_63)
    {
        float4 _66 = round(_10_testInputs);
        _73 = all(bool4(_66.x == expectedA.x, _66.y == expectedA.y, _66.z == expectedA.z, _66.w == expectedA.w));
    }
    else
    {
        _73 = false;
    }
    float4 _74 = 0.0f.xxxx;
    if (_73)
    {
        _74 = _10_colorGreen;
    }
    else
    {
        _74 = _10_colorRed;
    }
    return _74;
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
