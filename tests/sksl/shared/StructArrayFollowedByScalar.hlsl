struct S
{
    float rgb[3];
    float a;
};

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _18)
{
    S s = { { 0.0f, 0.0f, 0.0f }, 0.0f };
    s.rgb[0] = 0.0f;
    s.rgb[1] = 1.0f;
    s.rgb[2] = 0.0f;
    s.a = 1.0f;
    return float4(s.rgb[0], s.rgb[1], s.rgb[2], s.a);
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
