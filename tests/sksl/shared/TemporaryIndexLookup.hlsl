cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorGreen : packoffset(c0);
    float4 _8_colorRed : packoffset(c1);
    row_major float3x3 _8_testMatrix3x3 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float3x3 GetTestMatrix_f33()
{
    return _8_testMatrix3x3;
}

float4 main(float2 _31)
{
    float expected = 0.0f;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            float _55 = expected;
            float _57 = _55 + 1.0f;
            expected = _57;
            float3x3 _58 = GetTestMatrix_f33();
            if (_58[i][j] != _57)
            {
                return _8_colorRed;
            }
        }
    }
    return _8_colorGreen;
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
