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
    if (frac(_7_testInputs.x) == 0.75f)
    {
        float2 _36 = frac(_7_testInputs.xy);
        _44 = all(bool2(_36.x == float2(0.75f, 0.0f).x, _36.y == float2(0.75f, 0.0f).y));
    }
    else
    {
        _44 = false;
    }
    bool _56 = false;
    if (_44)
    {
        float3 _47 = frac(_7_testInputs.xyz);
        _56 = all(bool3(_47.x == float3(0.75f, 0.0f, 0.75f).x, _47.y == float3(0.75f, 0.0f, 0.75f).y, _47.z == float3(0.75f, 0.0f, 0.75f).z));
    }
    else
    {
        _56 = false;
    }
    bool _67 = false;
    if (_56)
    {
        float4 _59 = frac(_7_testInputs);
        _67 = all(bool4(_59.x == float4(0.75f, 0.0f, 0.75f, 0.25f).x, _59.y == float4(0.75f, 0.0f, 0.75f, 0.25f).y, _59.z == float4(0.75f, 0.0f, 0.75f, 0.25f).z, _59.w == float4(0.75f, 0.0f, 0.75f, 0.25f).w));
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
