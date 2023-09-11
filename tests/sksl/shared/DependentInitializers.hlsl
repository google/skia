cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float x = 0.5f;
    float _28 = 0.5f * 2.0f;
    float y = _28;
    float4 _32 = 0.0f.xxxx;
    if (_28 == 1.0f)
    {
        _32 = _7_colorGreen;
    }
    else
    {
        _32 = _7_colorRed;
    }
    return _32;
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
