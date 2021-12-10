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
    float2x2 a = float2x2(float3(1.0f, 0.0f, 0.0f).xy, float3(0.0f, 1.0f, 0.0f).xy);
    float _46 = 0.0f + a[0].x;
    result = _46;
    float2x2 b = float2x2(float4(1.0f, 0.0f, 0.0f, 0.0f).xy, float4(0.0f, 1.0f, 0.0f, 0.0f).xy);
    float _60 = _46 + b[0].x;
    result = _60;
    float3x3 c = float3x3(float4(1.0f, 0.0f, 0.0f, 0.0f).xyz, float4(0.0f, 1.0f, 0.0f, 0.0f).xyz, float4(0.0f, 0.0f, 1.0f, 0.0f).xyz);
    float _71 = _60 + c[0].x;
    result = _71;
    float3x3 d = float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
    float _79 = _71 + d[0].x;
    result = _79;
    float4x4 e = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float _86 = _79 + e[0].x;
    result = _86;
    float2x2 f = float2x2(float4(1.0f, 0.0f, 0.0f, 0.0f).xyz.xy, float4(0.0f, 1.0f, 0.0f, 0.0f).xyz.xy);
    float _98 = _86 + f[0].x;
    result = _98;
    float4 _101 = 0.0f.xxxx;
    if (_98 == 6.0f)
    {
        _101 = _10_colorGreen;
    }
    else
    {
        _101 = _10_colorRed;
    }
    return _101;
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
