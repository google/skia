struct S
{
    float2 v;
};

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void initialize_vS(inout S _24[2])
{
    _24[0].v = float2(0.0f, 1.0f);
    _24[1].v = float2(2.0f, 1.0f);
}

float4 main(float2 _35)
{
    float2 x[2] = { 0.0f.xx, 0.0f.xx };
    x[0] = 0.0f.xx;
    x[1] = float2(1.0f, 0.0f);
    float2 y[2] = { 0.0f.xx, 0.0f.xx };
    y[0] = float2(0.0f, 1.0f);
    y[1] = float2(-1.0f, 2.0f);
    S _49[2] = { { 0.0f.xx }, { 0.0f.xx } };
    initialize_vS(_49);
    S z[2] = _49;
    return float4((x[0].x * x[0].y) + z[0].v.x, x[1].x - (x[1].y * z[0].v.y), (y[0].x / y[0].y) / z[1].v.x, y[1].x + (y[1].y * z[1].v.y));
}

void frag_main()
{
    float2 _15 = 0.0f.xx;
    sk_FragColor = main(_15);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
