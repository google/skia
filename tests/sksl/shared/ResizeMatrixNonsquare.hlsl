cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float result = 0.0f;
    float3x3 g = float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
    float _45 = 0.0f + g[0].x;
    result = _45;
    float3x3 h = float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
    float _54 = _45 + h[0].x;
    result = _54;
    float4x4 i = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float _72 = _54 + i[0].x;
    result = _72;
    float4x4 j = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float _81 = _72 + j[0].x;
    result = _81;
    float2x4 k = float2x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f));
    float _87 = _81 + k[0].x;
    result = _87;
    float4x2 l = float4x2(float4(1.0f, 0.0f, 0.0f, 0.0f).xy, float4(0.0f, 1.0f, 0.0f, 0.0f).xy, 0.0f.xx, 0.0f.xx);
    float _96 = _87 + l[0].x;
    result = _96;
    float4 _100 = 0.0f.xxxx;
    if (_96 == 6.0f)
    {
        _100 = _11_colorGreen;
    }
    else
    {
        _100 = _11_colorRed;
    }
    return _100;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
