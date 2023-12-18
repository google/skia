static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    int x = 0;
    int y = 0;
    int z = 0;
    if (true)
    {
        x = 1;
    }
    if (false)
    {
        y = 1;
    }
    if (true)
    {
        z = 1;
    }
    float3 _33 = float3(float(x), float(y), float(z));
    sk_FragColor = float4(_33.x, _33.y, _33.z, sk_FragColor.w);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
