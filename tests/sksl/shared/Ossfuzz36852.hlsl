static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float2x2 x = float2x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f));
    float2 y = float4(x[0].x, x[0].y, x[1].x, x[1].y).xy;
    return y.xyxy;
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
