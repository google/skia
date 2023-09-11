static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _18)
{
    float4 x = 1.0f.xxxx;
    while (x.w == 1.0f)
    {
        x.x -= 0.25f;
        if (x.x <= 0.0f)
        {
            break;
        }
    }
    while (x.z > 0.0f)
    {
        x.z -= 0.25f;
        if (x.w == 1.0f)
        {
            continue;
        }
        x.y = 0.0f;
    }
    return x;
}

void frag_main()
{
    float2 _14 = 0.0f.xx;
    sk_FragColor = main(_14);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
