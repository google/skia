cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float _35[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float test1[4] = _35;
    float2 _42[2] = { float2(1.0f, 2.0f), float2(3.0f, 4.0f) };
    float2 test2[2] = _42;
    float4x4 _54[1] = { float4x4(float4(16.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 16.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 16.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 16.0f)) };
    float4x4 test3[1] = _54;
    float4 _71 = 0.0f.xxxx;
    if (((test1[3] + test2[1].y) + test3[0][3].w) == 24.0f)
    {
        _71 = _10_colorGreen;
    }
    else
    {
        _71 = _10_colorRed;
    }
    return _71;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
