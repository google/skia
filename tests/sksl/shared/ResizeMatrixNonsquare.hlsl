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
    float result = 0.0f;
    float3x3 g = float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
    float _42 = 0.0f + g[0].x;
    result = _42;
    float3x3 h = float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
    float _51 = _42 + h[0].x;
    result = _51;
    float4x4 i = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float _69 = _51 + i[0].x;
    result = _69;
    float4x4 j = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float _78 = _69 + j[0].x;
    result = _78;
    float2x4 k = float2x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f));
    float _84 = _78 + k[0].x;
    result = _84;
    float4x2 l = float4x2(float4(1.0f, 0.0f, 0.0f, 0.0f).xy, float4(0.0f, 1.0f, 0.0f, 0.0f).xy, 0.0f.xx, 0.0f.xx);
    float _93 = _84 + l[0].x;
    result = _93;
    float4 _97 = 0.0f.xxxx;
    if (_93 == 6.0f)
    {
        _97 = _7_colorGreen;
    }
    else
    {
        _97 = _7_colorRed;
    }
    return _97;
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
