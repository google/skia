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
    float result = 0.0f;
    float2x3 _33 = float2x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f));
    float3x3 g = float3x3(_33[0], _33[1], float3(0.0f, 0.0f, 1.0f));
    result += g[0].x;
    float3x2 _50 = float3x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f), 0.0f.xx);
    float3x3 h = float3x3(float3(_50[0], 0.0f), float3(_50[1], 0.0f), float3(_50[2], 1.0f));
    result += h[0].x;
    float4x2 _70 = float4x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f), 0.0f.xx, 0.0f.xx);
    float4x3 _76 = float4x3(float3(_70[0], 0.0f), float3(_70[1], 0.0f), float3(_70[2], 1.0f), float3(_70[3], 0.0f));
    float4x4 i = float4x4(float4(_76[0], 0.0f), float4(_76[1], 0.0f), float4(_76[2], 0.0f), float4(_76[3], 1.0f));
    result += i[0].x;
    float2x4 _102 = float2x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f));
    float3x4 _106 = float3x4(_102[0], _102[1], float4(0.0f, 0.0f, 1.0f, 0.0f));
    float4x4 j = float4x4(_106[0], _106[1], _106[2], float4(0.0f, 0.0f, 0.0f, 1.0f));
    result += j[0].x;
    float4x2 _123 = float4x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f), 0.0f.xx, 0.0f.xx);
    float2x4 k = float2x4(float4(_123[0], 0.0f, 0.0f), float4(_123[1], 0.0f, 0.0f));
    result += k[0].x;
    float2x4 _140 = float2x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f));
    float4x2 l = float4x2(_140[0].xy, _140[1].xy, 0.0f.xx, 0.0f.xx);
    result += l[0].x;
    float4 _158 = 0.0f.xxxx;
    if (result == 6.0f)
    {
        _158 = _10_colorGreen;
    }
    else
    {
        _158 = _10_colorRed;
    }
    return _158;
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
