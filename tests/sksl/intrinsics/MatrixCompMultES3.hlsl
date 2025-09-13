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
    float4 _41 = 9.0f.xxxx * _11_colorRed;
    float4 _42 = 9.0f.xxxx * _11_colorGreen;
    float2x4 h24 = float2x4(_41, _42);
    float2 _66 = float2(_11_colorRed.xy);
    float2 _69 = float2(_11_colorRed.zw);
    float2 _72 = float2(_11_colorGreen.xy);
    float2 _75 = float2(_11_colorGreen.zw);
    float2 _77 = float2(1.0f, 2.0f) * _66;
    float2 _78 = float2(3.0f, 4.0f) * _69;
    float2 _79 = float2(5.0f, 6.0f) * _72;
    float2 _80 = float2(7.0f, 8.0f) * _75;
    float4x2 h42 = float4x2(_77, _78, _79, _80);
    float4x3 f43 = float4x3(float3(12.0f, 22.0f, 30.0f), float3(36.0f, 40.0f, 42.0f), float3(42.0f, 40.0f, 36.0f), float3(30.0f, 22.0f, 12.0f));
    bool _127 = false;
    if (all(bool4(_41.x == float4(9.0f, 0.0f, 0.0f, 9.0f).x, _41.y == float4(9.0f, 0.0f, 0.0f, 9.0f).y, _41.z == float4(9.0f, 0.0f, 0.0f, 9.0f).z, _41.w == float4(9.0f, 0.0f, 0.0f, 9.0f).w)) && all(bool4(_42.x == float4(0.0f, 9.0f, 0.0f, 9.0f).x, _42.y == float4(0.0f, 9.0f, 0.0f, 9.0f).y, _42.z == float4(0.0f, 9.0f, 0.0f, 9.0f).z, _42.w == float4(0.0f, 9.0f, 0.0f, 9.0f).w)))
    {
        _127 = ((all(bool2(_77.x == float2(1.0f, 0.0f).x, _77.y == float2(1.0f, 0.0f).y)) && all(bool2(_78.x == float2(0.0f, 4.0f).x, _78.y == float2(0.0f, 4.0f).y))) && all(bool2(_79.x == float2(0.0f, 6.0f).x, _79.y == float2(0.0f, 6.0f).y))) && all(bool2(_80.x == float2(0.0f, 8.0f).x, _80.y == float2(0.0f, 8.0f).y));
    }
    else
    {
        _127 = false;
    }
    bool _142 = false;
    if (_127)
    {
        _142 = ((all(bool3(float3(12.0f, 22.0f, 30.0f).x == float3(12.0f, 22.0f, 30.0f).x, float3(12.0f, 22.0f, 30.0f).y == float3(12.0f, 22.0f, 30.0f).y, float3(12.0f, 22.0f, 30.0f).z == float3(12.0f, 22.0f, 30.0f).z)) && all(bool3(float3(36.0f, 40.0f, 42.0f).x == float3(36.0f, 40.0f, 42.0f).x, float3(36.0f, 40.0f, 42.0f).y == float3(36.0f, 40.0f, 42.0f).y, float3(36.0f, 40.0f, 42.0f).z == float3(36.0f, 40.0f, 42.0f).z))) && all(bool3(float3(42.0f, 40.0f, 36.0f).x == float3(42.0f, 40.0f, 36.0f).x, float3(42.0f, 40.0f, 36.0f).y == float3(42.0f, 40.0f, 36.0f).y, float3(42.0f, 40.0f, 36.0f).z == float3(42.0f, 40.0f, 36.0f).z))) && all(bool3(float3(30.0f, 22.0f, 12.0f).x == float3(30.0f, 22.0f, 12.0f).x, float3(30.0f, 22.0f, 12.0f).y == float3(30.0f, 22.0f, 12.0f).y, float3(30.0f, 22.0f, 12.0f).z == float3(30.0f, 22.0f, 12.0f).z));
    }
    else
    {
        _142 = false;
    }
    float4 _143 = 0.0f.xxxx;
    if (_142)
    {
        _143 = _11_colorGreen;
    }
    else
    {
        _143 = _11_colorRed;
    }
    return _143;
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
