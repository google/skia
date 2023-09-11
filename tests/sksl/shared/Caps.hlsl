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
    float3 _21 = float3(float(1), float(0), float(1));
    sk_FragColor = float4(_21.x, _21.y, _21.z, sk_FragColor.w);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
