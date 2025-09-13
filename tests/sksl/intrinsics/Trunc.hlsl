cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool _47 = false;
    if (trunc(_11_testInputs.x) == (-1.0f))
    {
        float2 _39 = trunc(_11_testInputs.xy);
        _47 = all(bool2(_39.x == float2(-1.0f, 0.0f).x, _39.y == float2(-1.0f, 0.0f).y));
    }
    else
    {
        _47 = false;
    }
    bool _59 = false;
    if (_47)
    {
        float3 _50 = trunc(_11_testInputs.xyz);
        _59 = all(bool3(_50.x == float3(-1.0f, 0.0f, 0.0f).x, _50.y == float3(-1.0f, 0.0f, 0.0f).y, _50.z == float3(-1.0f, 0.0f, 0.0f).z));
    }
    else
    {
        _59 = false;
    }
    bool _70 = false;
    if (_59)
    {
        float4 _62 = trunc(_11_testInputs);
        _70 = all(bool4(_62.x == float4(-1.0f, 0.0f, 0.0f, 2.0f).x, _62.y == float4(-1.0f, 0.0f, 0.0f, 2.0f).y, _62.z == float4(-1.0f, 0.0f, 0.0f, 2.0f).z, _62.w == float4(-1.0f, 0.0f, 0.0f, 2.0f).w));
    }
    else
    {
        _70 = false;
    }
    float4 _71 = 0.0f.xxxx;
    if (_70)
    {
        _71 = _11_colorGreen;
    }
    else
    {
        _71 = _11_colorRed;
    }
    return _71;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
