static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(inout float2 _18)
{
    _18 *= 0.3333333432674407958984375f;
    return 1.0f.xxxx;
}

void frag_main()
{
    float2 _14 = 0.0f.xx;
    float4 _16 = main(_14);
    sk_FragColor = _16;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
