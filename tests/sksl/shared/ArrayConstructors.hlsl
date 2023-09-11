cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float _32[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float test1[4] = _32;
    float2 _39[2] = { float2(1.0f, 2.0f), float2(3.0f, 4.0f) };
    float2 test2[2] = _39;
    float4x4 _51[1] = { float4x4(float4(16.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 16.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 16.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 16.0f)) };
    float4x4 test3[1] = _51;
    float4 _69 = 0.0f.xxxx;
    if (((test1[3] + test2[1].y) + test3[0][3].w) == 24.0f)
    {
        _69 = _7_colorGreen;
    }
    else
    {
        _69 = _7_colorRed;
    }
    return _69;
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
