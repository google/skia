cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _10_testMatrix2x2 : packoffset(c0);
    row_major float3x3 _10_testMatrix3x3 : packoffset(c2);
    float4 _10_colorGreen : packoffset(c5);
    float4 _10_colorRed : packoffset(c6);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    float2x3 testMatrix2x3 = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
    float2x2 _42 = transpose(_10_testMatrix2x2);
    float2x2 _50 = float2x2(float2(1.0f, 3.0f), float2(2.0f, 4.0f));
    float2 _52 = _42[0];
    float2 _53 = _50[0];
    float2 _56 = _42[1];
    float2 _57 = _50[1];
    bool _84 = false;
    if (all(bool2(_52.x == _53.x, _52.y == _53.y)) && all(bool2(_56.x == _57.x, _56.y == _57.y)))
    {
        float3x2 _63 = transpose(testMatrix2x3);
        float3x2 _69 = float3x2(float2(1.0f, 4.0f), float2(2.0f, 5.0f), float2(3.0f, 6.0f));
        float2 _70 = _63[0];
        float2 _71 = _69[0];
        float2 _74 = _63[1];
        float2 _75 = _69[1];
        float2 _79 = _63[2];
        float2 _80 = _69[2];
        _84 = (all(bool2(_70.x == _71.x, _70.y == _71.y)) && all(bool2(_74.x == _75.x, _74.y == _75.y))) && all(bool2(_79.x == _80.x, _79.y == _80.y));
    }
    else
    {
        _84 = false;
    }
    bool _114 = false;
    if (_84)
    {
        float3x3 _87 = transpose(_10_testMatrix3x3);
        float3x3 _98 = float3x3(float3(1.0f, 4.0f, 7.0f), float3(2.0f, 5.0f, 8.0f), float3(3.0f, 6.0f, 9.0f));
        float3 _100 = _87[0];
        float3 _101 = _98[0];
        float3 _104 = _87[1];
        float3 _105 = _98[1];
        float3 _109 = _87[2];
        float3 _110 = _98[2];
        _114 = (all(bool3(_100.x == _101.x, _100.y == _101.y, _100.z == _101.z)) && all(bool3(_104.x == _105.x, _104.y == _105.y, _104.z == _105.z))) && all(bool3(_109.x == _110.x, _109.y == _110.y, _109.z == _110.z));
    }
    else
    {
        _114 = false;
    }
    float4 _115 = 0.0f.xxxx;
    if (_114)
    {
        _115 = _10_colorGreen;
    }
    else
    {
        _115 = _10_colorRed;
    }
    return _115;
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
