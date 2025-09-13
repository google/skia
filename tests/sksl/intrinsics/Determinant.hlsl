cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _11_testMatrix2x2 : packoffset(c0);
    float4 _11_colorGreen : packoffset(c2);
    float4 _11_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    float4 _36 = 0.0f.xxxx;
    if (determinant(_11_testMatrix2x2) == (-2.0f))
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
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
