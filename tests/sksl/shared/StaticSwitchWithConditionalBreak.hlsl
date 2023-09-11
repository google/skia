cbuffer _UniformBuffer : register(b0, space0)
{
    float _7_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float value = 0.0f;
    switch (0)
    {
        case 0:
        {
            value = 0.0f;
            if (_7_unknownInput == 2.0f)
            {
                break;
            }
            value = 1.0f;
            break;
        }
        case 1:
        {
            value = 1.0f;
            break;
        }
    }
    sk_FragColor = value.xxxx;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
