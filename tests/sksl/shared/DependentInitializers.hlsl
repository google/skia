cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float x = 0.5f;
    float _32 = 0.5f * 2.0f;
    float y = _32;
    float4 _36 = 0.0f.xxxx;
    if (_32 == 1.0f)
    {
        _36 = _11_colorGreen;
    }
    else
    {
        _36 = _11_colorRed;
    }
    return _36;
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
