static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float b = 2.0f;
    float c = 3.0f;
    for (int x = 0; x < 1; x++)
    {
    }
    float d = c;
    b += 1.0f;
    d += 1.0f;
    return float4(float(b == 2.0f), float(b == 3.0f), float(d == 5.0f), float(d == 4.0f));
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
