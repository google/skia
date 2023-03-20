cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float x = 0.5f;
    float _31 = 0.5f * 2.0f;
    float y = _31;
    float4 _34 = 0.0f.xxxx;
    if (_31 == 1.0f)
    {
        _34 = _10_colorGreen;
    }
    else
    {
        _34 = _10_colorRed;
    }
    return _34;
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
