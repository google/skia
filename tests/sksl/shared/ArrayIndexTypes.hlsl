static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float _19[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float array[4] = _19;
    int x = 0;
    uint y = 1u;
    int z = 2;
    uint w = 3u;
    sk_FragColor = float4(array[0], array[1u], array[2], array[3u]);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
