cbuffer _UniformBuffer : register(b0, space0)
{
    float _7_testArray[5] : packoffset(c0);
    float4 _7_colorGreen : packoffset(c5);
    float4 _7_colorRed : packoffset(c6);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    for (int index = 0; index < 5; index++)
    {
        if (_7_testArray[index] != float(index + 1))
        {
            return _7_colorRed;
        }
    }
    return _7_colorGreen;
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
