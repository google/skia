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
    float4 _39 = 9.0f.xxxx * _7_colorRed;
    float4 _40 = 9.0f.xxxx * _7_colorGreen;
    float2x4 h24 = float2x4(_39, _40);
    float2 _65 = float2(_7_colorRed.xy);
    float2 _68 = float2(_7_colorRed.zw);
    float2 _71 = float2(_7_colorGreen.xy);
    float2 _74 = float2(_7_colorGreen.zw);
    float2 _76 = float2(1.0f, 2.0f) * _65;
    float2 _77 = float2(3.0f, 4.0f) * _68;
    float2 _78 = float2(5.0f, 6.0f) * _71;
    float2 _79 = float2(7.0f, 8.0f) * _74;
    float4x2 h42 = float4x2(_76, _77, _78, _79);
    float4x3 f43 = float4x3(float3(12.0f, 22.0f, 30.0f), float3(36.0f, 40.0f, 42.0f), float3(42.0f, 40.0f, 36.0f), float3(30.0f, 22.0f, 12.0f));
    bool _126 = false;
    if (all(bool4(_39.x == float4(9.0f, 0.0f, 0.0f, 9.0f).x, _39.y == float4(9.0f, 0.0f, 0.0f, 9.0f).y, _39.z == float4(9.0f, 0.0f, 0.0f, 9.0f).z, _39.w == float4(9.0f, 0.0f, 0.0f, 9.0f).w)) && all(bool4(_40.x == float4(0.0f, 9.0f, 0.0f, 9.0f).x, _40.y == float4(0.0f, 9.0f, 0.0f, 9.0f).y, _40.z == float4(0.0f, 9.0f, 0.0f, 9.0f).z, _40.w == float4(0.0f, 9.0f, 0.0f, 9.0f).w)))
    {
        _126 = ((all(bool2(_76.x == float2(1.0f, 0.0f).x, _76.y == float2(1.0f, 0.0f).y)) && all(bool2(_77.x == float2(0.0f, 4.0f).x, _77.y == float2(0.0f, 4.0f).y))) && all(bool2(_78.x == float2(0.0f, 6.0f).x, _78.y == float2(0.0f, 6.0f).y))) && all(bool2(_79.x == float2(0.0f, 8.0f).x, _79.y == float2(0.0f, 8.0f).y));
    }
    else
    {
        _126 = false;
    }
    bool _141 = false;
    if (_126)
    {
        _141 = ((all(bool3(float3(12.0f, 22.0f, 30.0f).x == float3(12.0f, 22.0f, 30.0f).x, float3(12.0f, 22.0f, 30.0f).y == float3(12.0f, 22.0f, 30.0f).y, float3(12.0f, 22.0f, 30.0f).z == float3(12.0f, 22.0f, 30.0f).z)) && all(bool3(float3(36.0f, 40.0f, 42.0f).x == float3(36.0f, 40.0f, 42.0f).x, float3(36.0f, 40.0f, 42.0f).y == float3(36.0f, 40.0f, 42.0f).y, float3(36.0f, 40.0f, 42.0f).z == float3(36.0f, 40.0f, 42.0f).z))) && all(bool3(float3(42.0f, 40.0f, 36.0f).x == float3(42.0f, 40.0f, 36.0f).x, float3(42.0f, 40.0f, 36.0f).y == float3(42.0f, 40.0f, 36.0f).y, float3(42.0f, 40.0f, 36.0f).z == float3(42.0f, 40.0f, 36.0f).z))) && all(bool3(float3(30.0f, 22.0f, 12.0f).x == float3(30.0f, 22.0f, 12.0f).x, float3(30.0f, 22.0f, 12.0f).y == float3(30.0f, 22.0f, 12.0f).y, float3(30.0f, 22.0f, 12.0f).z == float3(30.0f, 22.0f, 12.0f).z));
    }
    else
    {
        _141 = false;
    }
    float4 _142 = 0.0f.xxxx;
    if (_141)
    {
        _142 = _7_colorGreen;
    }
    else
    {
        _142 = _7_colorRed;
    }
    return _142;
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
