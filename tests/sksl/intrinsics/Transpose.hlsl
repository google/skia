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
    float2 _52 = _42[0];
    float2 _55 = _42[1];
    bool _78 = false;
    if (all(bool2(_52.x == float2(1.0f, 3.0f).x, _52.y == float2(1.0f, 3.0f).y)) && all(bool2(_55.x == float2(2.0f, 4.0f).x, _55.y == float2(2.0f, 4.0f).y)))
    {
        float3x2 _61 = transpose(float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f)));
        float2 _67 = _61[0];
        float2 _70 = _61[1];
        float2 _74 = _61[2];
        _78 = (all(bool2(_67.x == float2(1.0f, 4.0f).x, _67.y == float2(1.0f, 4.0f).y)) && all(bool2(_70.x == float2(2.0f, 5.0f).x, _70.y == float2(2.0f, 5.0f).y))) && all(bool2(_74.x == float2(3.0f, 6.0f).x, _74.y == float2(3.0f, 6.0f).y));
    }
    else
    {
        _78 = false;
    }
    bool _105 = false;
    if (_78)
    {
        float3x3 _81 = transpose(_10_testMatrix3x3);
        float3 _94 = _81[0];
        float3 _97 = _81[1];
        float3 _101 = _81[2];
        _105 = (all(bool3(_94.x == float3(1.0f, 4.0f, 7.0f).x, _94.y == float3(1.0f, 4.0f, 7.0f).y, _94.z == float3(1.0f, 4.0f, 7.0f).z)) && all(bool3(_97.x == float3(2.0f, 5.0f, 8.0f).x, _97.y == float3(2.0f, 5.0f, 8.0f).y, _97.z == float3(2.0f, 5.0f, 8.0f).z))) && all(bool3(_101.x == float3(3.0f, 6.0f, 9.0f).x, _101.y == float3(3.0f, 6.0f, 9.0f).y, _101.z == float3(3.0f, 6.0f, 9.0f).z));
    }
    else
    {
        _105 = false;
    }
    float4 _106 = 0.0f.xxxx;
    if (_105)
    {
        _106 = _10_colorGreen;
    }
    else
    {
        _106 = _10_colorRed;
    }
    return _106;
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
