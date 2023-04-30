static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(inout float2 _21)
{
    _21 *= 0.3333333432674407958984375f;
    return 1.0f.xxxx;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    float4 _19 = main(_17);
    sk_FragColor = _19;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
