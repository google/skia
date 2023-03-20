cbuffer _UniformBuffer : register(b0, space0)
{
    float _10_testArray[5] : packoffset(c0);
    float4 _10_colorGreen : packoffset(c5);
    float4 _10_colorRed : packoffset(c6);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    for (int index = 0; index < 5; index++)
    {
        if (_10_testArray[index] != float(index + 1))
        {
            return _10_colorRed;
        }
    }
    return _10_colorGreen;
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
