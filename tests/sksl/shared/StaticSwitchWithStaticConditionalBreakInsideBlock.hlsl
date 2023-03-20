static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float x = 0.0f;
    switch (0)
    {
        case 0:
        {
            x = 0.0f;
            if (0.0f < 1.0f)
            {
                sk_FragColor = 0.0f.xxxx;
                break;
            }
            x = 1.0f;
            break;
        }
        case 1:
        {
            x = 1.0f;
            break;
        }
    }
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
