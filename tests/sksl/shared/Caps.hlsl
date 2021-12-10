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
    x = 1;
    z = 1;
    float3 _26 = float3(float(x), float(y), float(z));
    sk_FragColor = float4(_26.x, _26.y, _26.z, sk_FragColor.w);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
