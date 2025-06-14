cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _11_testMatrix2x2 : packoffset(c0);
    row_major float3x3 _11_testMatrix3x3 : packoffset(c2);
    float4 _11_colorGreen : packoffset(c5);
    float4 _11_colorRed : packoffset(c6);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _28)
{
    float2x3 testMatrix2x3 = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
    float2x2 _44 = transpose(_11_testMatrix2x2);
    float2 _53 = _44[0];
    float2 _56 = _44[1];
    bool _79 = false;
    if (all(bool2(_53.x == float2(1.0f, 3.0f).x, _53.y == float2(1.0f, 3.0f).y)) && all(bool2(_56.x == float2(2.0f, 4.0f).x, _56.y == float2(2.0f, 4.0f).y)))
    {
        float3x2 _62 = transpose(float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f)));
        float2 _68 = _62[0];
        float2 _71 = _62[1];
        float2 _75 = _62[2];
        _79 = (all(bool2(_68.x == float2(1.0f, 4.0f).x, _68.y == float2(1.0f, 4.0f).y)) && all(bool2(_71.x == float2(2.0f, 5.0f).x, _71.y == float2(2.0f, 5.0f).y))) && all(bool2(_75.x == float2(3.0f, 6.0f).x, _75.y == float2(3.0f, 6.0f).y));
    }
    else
    {
        _79 = false;
    }
    bool _106 = false;
    if (_79)
    {
        float3x3 _82 = transpose(_11_testMatrix3x3);
        float3 _95 = _82[0];
        float3 _98 = _82[1];
        float3 _102 = _82[2];
        _106 = (all(bool3(_95.x == float3(1.0f, 4.0f, 7.0f).x, _95.y == float3(1.0f, 4.0f, 7.0f).y, _95.z == float3(1.0f, 4.0f, 7.0f).z)) && all(bool3(_98.x == float3(2.0f, 5.0f, 8.0f).x, _98.y == float3(2.0f, 5.0f, 8.0f).y, _98.z == float3(2.0f, 5.0f, 8.0f).z))) && all(bool3(_102.x == float3(3.0f, 6.0f, 9.0f).x, _102.y == float3(3.0f, 6.0f, 9.0f).y, _102.z == float3(3.0f, 6.0f, 9.0f).z));
    }
    else
    {
        _106 = false;
    }
    float4 _107 = 0.0f.xxxx;
    if (_106)
    {
        _107 = _11_colorGreen;
    }
    else
    {
        _107 = _11_colorRed;
    }
    return _107;
}

void frag_main()
{
    float2 _24 = 0.0f.xx;
    sk_FragColor = main(_24);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
