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
    float2x2 a = float2x2(float3(1.0f, 0.0f, 0.0f).xy, float3(0.0f, 1.0f, 0.0f).xy);
    float _43 = 0.0f + a[0].x;
    result = _43;
    float2x2 b = float2x2(float4(1.0f, 0.0f, 0.0f, 0.0f).xy, float4(0.0f, 1.0f, 0.0f, 0.0f).xy);
    float _57 = _43 + b[0].x;
    result = _57;
    float3x3 c = float3x3(float4(1.0f, 0.0f, 0.0f, 0.0f).xyz, float4(0.0f, 1.0f, 0.0f, 0.0f).xyz, float4(0.0f, 0.0f, 1.0f, 0.0f).xyz);
    float _68 = _57 + c[0].x;
    result = _68;
    float3x3 d = float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
    float _76 = _68 + d[0].x;
    result = _76;
    float4x4 e = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float _83 = _76 + e[0].x;
    result = _83;
    float2x2 f = float2x2(float4(1.0f, 0.0f, 0.0f, 0.0f).xyz.xy, float4(0.0f, 1.0f, 0.0f, 0.0f).xyz.xy);
    float _95 = _83 + f[0].x;
    result = _95;
    float4 _99 = 0.0f.xxxx;
    if (_95 == 6.0f)
    {
        _99 = _7_colorGreen;
    }
    else
    {
        _99 = _7_colorRed;
    }
    return _99;
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
