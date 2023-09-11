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
    bool _44 = false;
    if (round(_7_testInputs.x) == (-1.0f))
    {
        float2 _36 = round(_7_testInputs.xy);
        _44 = all(bool2(_36.x == float2(-1.0f, 0.0f).x, _36.y == float2(-1.0f, 0.0f).y));
    }
    else
    {
        _44 = false;
    }
    bool _57 = false;
    if (_44)
    {
        float3 _47 = round(_7_testInputs.xyz);
        _57 = all(bool3(_47.x == float3(-1.0f, 0.0f, 1.0f).x, _47.y == float3(-1.0f, 0.0f, 1.0f).y, _47.z == float3(-1.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _57 = false;
    }
    bool _68 = false;
    if (_57)
    {
        float4 _60 = round(_7_testInputs);
        _68 = all(bool4(_60.x == float4(-1.0f, 0.0f, 1.0f, 2.0f).x, _60.y == float4(-1.0f, 0.0f, 1.0f, 2.0f).y, _60.z == float4(-1.0f, 0.0f, 1.0f, 2.0f).z, _60.w == float4(-1.0f, 0.0f, 1.0f, 2.0f).w));
    }
    else
    {
        _68 = false;
    }
    float4 _69 = 0.0f.xxxx;
    if (_68)
    {
        _69 = _7_colorGreen;
    }
    else
    {
        _69 = _7_colorRed;
    }
    return _69;
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
