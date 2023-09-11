cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorWhite : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 x = _7_colorWhite;
    for (float r = -5.0f; r < 5.0f; r += 1.0f)
    {
        x.x = clamp(r, 0.0f, 1.0f);
        if (x.x == 0.0f)
        {
            break;
        }
    }
    for (float b = 5.0f; b >= 0.0f; b -= 1.0f)
    {
        x.z = b;
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
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
