cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    row_major float3x3 _11_testMatrix3x3 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float3x3 GetTestMatrix_f33()
{
    return _11_testMatrix3x3;
}

float4 main(float2 _34)
{
    float expected = 0.0f;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            float _57 = expected;
            float _59 = _57 + 1.0f;
            expected = _59;
            float3x3 _60 = GetTestMatrix_f33();
            if (_60[i][j] != _59)
            {
                return _11_colorRed;
            }
        }
    }
    return _11_colorGreen;
}

void frag_main()
{
    float2 _23 = 0.0f.xx;
    sk_FragColor = main(_23);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
