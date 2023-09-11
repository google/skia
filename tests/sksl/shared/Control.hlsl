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
    if (_7_unknownInput > 5.0f)
    {
        sk_FragColor = 0.75f.xxxx;
    }
    else
    {
        discard;
    }
    int i = 0;
    while (i < 10)
    {
        sk_FragColor *= 0.5f;
        i++;
    }
    do
    {
        sk_FragColor += 0.25f.xxxx;
    } while (sk_FragColor.x < 0.75f);
    for (int i_1 = 0; i_1 < 10; i_1++)
    {
        if ((i_1 % 2) == 1)
        {
            break;
        }
        else
        {
            if (i_1 > 100)
            {
                return;
            }
            else
            {
                continue;
            }
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
