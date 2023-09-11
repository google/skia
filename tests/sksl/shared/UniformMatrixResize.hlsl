cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float3x3 _8_testMatrix3x3 : packoffset(c0);
    float4 _8_colorGreen : packoffset(c3);
    float4 _8_colorRed : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float2x2 resizeMatrix_f22()
{
    return float2x2(_8_testMatrix3x3[0].xy, _8_testMatrix3x3[1].xy);
}

float4 main(float2 _38)
{
    float2x2 _42 = resizeMatrix_f22();
    float2 _50 = _42[0];
    float2 _53 = _42[1];
    bool _78 = false;
    if (all(bool2(_50.x == float2(1.0f, 2.0f).x, _50.y == float2(1.0f, 2.0f).y)) && all(bool2(_53.x == float2(4.0f, 5.0f).x, _53.y == float2(4.0f, 5.0f).y)))
    {
        float2x2 _59 = resizeMatrix_f22();
        float3 _61 = float3(_59[0], 0.0f);
        float3 _63 = float3(_59[1], 0.0f);
        _78 = (all(bool3(_61.x == float3(1.0f, 2.0f, 0.0f).x, _61.y == float3(1.0f, 2.0f, 0.0f).y, _61.z == float3(1.0f, 2.0f, 0.0f).z)) && all(bool3(_63.x == float3(4.0f, 5.0f, 0.0f).x, _63.y == float3(4.0f, 5.0f, 0.0f).y, _63.z == float3(4.0f, 5.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 1.0f).x == float3(0.0f, 0.0f, 1.0f).x, float3(0.0f, 0.0f, 1.0f).y == float3(0.0f, 0.0f, 1.0f).y, float3(0.0f, 0.0f, 1.0f).z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _78 = false;
    }
    float4 _79 = 0.0f.xxxx;
    if (_78)
    {
        _79 = _8_colorGreen;
    }
    else
    {
        _79 = _8_colorRed;
    }
    return _79;
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
