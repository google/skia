struct Test
{
    int x;
    int y;
    int z;
};

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    Test t = { 0, 0, 0 };
    t.x = 0;
    sk_FragColor.x = float(t.x);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
