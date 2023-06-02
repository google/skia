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
    float4 _42 = 9.0f.xxxx * _10_colorRed;
    float4 _43 = 9.0f.xxxx * _10_colorGreen;
    float2x4 h24 = float2x4(_42, _43);
    float2 _68 = float2(_10_colorRed.xy);
    float2 _71 = float2(_10_colorRed.zw);
    float2 _74 = float2(_10_colorGreen.xy);
    float2 _77 = float2(_10_colorGreen.zw);
    float2 _79 = float2(1.0f, 2.0f) * _68;
    float2 _80 = float2(3.0f, 4.0f) * _71;
    float2 _81 = float2(5.0f, 6.0f) * _74;
    float2 _82 = float2(7.0f, 8.0f) * _77;
    float4x2 h42 = float4x2(_79, _80, _81, _82);
    float4x3 f43 = float4x3(float3(12.0f, 22.0f, 30.0f), float3(36.0f, 40.0f, 42.0f), float3(42.0f, 40.0f, 36.0f), float3(30.0f, 22.0f, 12.0f));
    bool _128 = false;
    if (all(bool4(_42.x == float4(9.0f, 0.0f, 0.0f, 9.0f).x, _42.y == float4(9.0f, 0.0f, 0.0f, 9.0f).y, _42.z == float4(9.0f, 0.0f, 0.0f, 9.0f).z, _42.w == float4(9.0f, 0.0f, 0.0f, 9.0f).w)) && all(bool4(_43.x == float4(0.0f, 9.0f, 0.0f, 9.0f).x, _43.y == float4(0.0f, 9.0f, 0.0f, 9.0f).y, _43.z == float4(0.0f, 9.0f, 0.0f, 9.0f).z, _43.w == float4(0.0f, 9.0f, 0.0f, 9.0f).w)))
    {
        _128 = ((all(bool2(_79.x == float2(1.0f, 0.0f).x, _79.y == float2(1.0f, 0.0f).y)) && all(bool2(_80.x == float2(0.0f, 4.0f).x, _80.y == float2(0.0f, 4.0f).y))) && all(bool2(_81.x == float2(0.0f, 6.0f).x, _81.y == float2(0.0f, 6.0f).y))) && all(bool2(_82.x == float2(0.0f, 8.0f).x, _82.y == float2(0.0f, 8.0f).y));
    }
    else
    {
        _128 = false;
    }
    bool _143 = false;
    if (_128)
    {
        _143 = ((all(bool3(float3(12.0f, 22.0f, 30.0f).x == float3(12.0f, 22.0f, 30.0f).x, float3(12.0f, 22.0f, 30.0f).y == float3(12.0f, 22.0f, 30.0f).y, float3(12.0f, 22.0f, 30.0f).z == float3(12.0f, 22.0f, 30.0f).z)) && all(bool3(float3(36.0f, 40.0f, 42.0f).x == float3(36.0f, 40.0f, 42.0f).x, float3(36.0f, 40.0f, 42.0f).y == float3(36.0f, 40.0f, 42.0f).y, float3(36.0f, 40.0f, 42.0f).z == float3(36.0f, 40.0f, 42.0f).z))) && all(bool3(float3(42.0f, 40.0f, 36.0f).x == float3(42.0f, 40.0f, 36.0f).x, float3(42.0f, 40.0f, 36.0f).y == float3(42.0f, 40.0f, 36.0f).y, float3(42.0f, 40.0f, 36.0f).z == float3(42.0f, 40.0f, 36.0f).z))) && all(bool3(float3(30.0f, 22.0f, 12.0f).x == float3(30.0f, 22.0f, 12.0f).x, float3(30.0f, 22.0f, 12.0f).y == float3(30.0f, 22.0f, 12.0f).y, float3(30.0f, 22.0f, 12.0f).z == float3(30.0f, 22.0f, 12.0f).z));
    }
    else
    {
        _143 = false;
    }
    float4 _144 = 0.0f.xxxx;
    if (_143)
    {
        _144 = _10_colorGreen;
    }
    else
    {
        _144 = _10_colorRed;
    }
    return _144;
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
