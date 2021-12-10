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
    float3x3 _32 = float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
    float2x2 a = float2x2(_32[0].xy, _32[1].xy);
    result += a[0].x;
    float4x4 _51 = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float2x2 b = float2x2(_51[0].xy, _51[1].xy);
    result += b[0].x;
    float4x4 _69 = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float3x3 c = float3x3(_69[0].xyz, _69[1].xyz, _69[2].xyz);
    result += c[0].x;
    float2x2 _88 = float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f));
    float3x3 d = float3x3(float3(_88[0], 0.0f), float3(_88[1], 0.0f), float3(0.0f, 0.0f, 1.0f));
    result += d[0].x;
    float2x2 _104 = float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f));
    float3x3 _107 = float3x3(float3(_104[0], 0.0f), float3(_104[1], 0.0f), float3(0.0f, 0.0f, 1.0f));
    float4x4 e = float4x4(float4(_107[0], 0.0f), float4(_107[1], 0.0f), float4(_107[2], 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    result += e[0].x;
    float4x4 _128 = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float3x3 _133 = float3x3(_128[0].xyz, _128[1].xyz, _128[2].xyz);
    float2x2 f = float2x2(_133[0].xy, _133[1].xy);
    result += f[0].x;
    float4 _153 = 0.0f.xxxx;
    if (result == 6.0f)
    {
        _153 = _10_colorGreen;
    }
    else
    {
        _153 = _10_colorRed;
    }
    return _153;
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
