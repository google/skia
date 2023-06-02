cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c2);
    row_major float3x3 _10_testMatrix3x3 : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    float2x2 h22 = float2x2(float2(0.0f, 5.0f), float2(10.0f, 15.0f));
    float2 _49 = _10_testMatrix2x2[0] * float2(1.0f, 0.0f);
    float2 _51 = _10_testMatrix2x2[1] * float2(0.0f, 1.0f);
    float2x2 f22 = float2x2(_49, _51);
    float3 _64 = _10_testMatrix3x3[0] * 2.0f.xxx;
    float3 _66 = _10_testMatrix3x3[1] * 2.0f.xxx;
    float3 _68 = _10_testMatrix3x3[2] * 2.0f.xxx;
    float3x3 h33 = float3x3(_64, _66, _68);
    bool _87 = false;
    if (all(bool2(float2(0.0f, 5.0f).x == float2(0.0f, 5.0f).x, float2(0.0f, 5.0f).y == float2(0.0f, 5.0f).y)) && all(bool2(float2(10.0f, 15.0f).x == float2(10.0f, 15.0f).x, float2(10.0f, 15.0f).y == float2(10.0f, 15.0f).y)))
    {
        _87 = all(bool2(_49.x == float2(1.0f, 0.0f).x, _49.y == float2(1.0f, 0.0f).y)) && all(bool2(_51.x == float2(0.0f, 4.0f).x, _51.y == float2(0.0f, 4.0f).y));
    }
    else
    {
        _87 = false;
    }
    bool _109 = false;
    if (_87)
    {
        _109 = (all(bool3(_64.x == float3(2.0f, 4.0f, 6.0f).x, _64.y == float3(2.0f, 4.0f, 6.0f).y, _64.z == float3(2.0f, 4.0f, 6.0f).z)) && all(bool3(_66.x == float3(8.0f, 10.0f, 12.0f).x, _66.y == float3(8.0f, 10.0f, 12.0f).y, _66.z == float3(8.0f, 10.0f, 12.0f).z))) && all(bool3(_68.x == float3(14.0f, 16.0f, 18.0f).x, _68.y == float3(14.0f, 16.0f, 18.0f).y, _68.z == float3(14.0f, 16.0f, 18.0f).z));
    }
    else
    {
        _109 = false;
    }
    float4 _110 = 0.0f.xxxx;
    if (_109)
    {
        _110 = _10_colorGreen;
    }
    else
    {
        _110 = _10_colorRed;
    }
    return _110;
}

void frag_main()
{
    float2 _23 = 0.0f.xx;
    sk_FragColor = main(_23);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
