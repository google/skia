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
    if (trunc(_7_testInputs.x) == (-1.0f))
    {
        float2 _36 = trunc(_7_testInputs.xy);
        _44 = all(bool2(_36.x == float2(-1.0f, 0.0f).x, _36.y == float2(-1.0f, 0.0f).y));
    }
    else
    {
        _44 = false;
    }
    bool _56 = false;
    if (_44)
    {
        float3 _47 = trunc(_7_testInputs.xyz);
        _56 = all(bool3(_47.x == float3(-1.0f, 0.0f, 0.0f).x, _47.y == float3(-1.0f, 0.0f, 0.0f).y, _47.z == float3(-1.0f, 0.0f, 0.0f).z));
    }
    else
    {
        _56 = false;
    }
    bool _67 = false;
    if (_56)
    {
        float4 _59 = trunc(_7_testInputs);
        _67 = all(bool4(_59.x == float4(-1.0f, 0.0f, 0.0f, 2.0f).x, _59.y == float4(-1.0f, 0.0f, 0.0f, 2.0f).y, _59.z == float4(-1.0f, 0.0f, 0.0f, 2.0f).z, _59.w == float4(-1.0f, 0.0f, 0.0f, 2.0f).w));
    }
    else
    {
        _67 = false;
    }
    float4 _68 = 0.0f.xxxx;
    if (_67)
    {
        _68 = _7_colorGreen;
    }
    else
    {
        _68 = _7_colorRed;
    }
    return _68;
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
