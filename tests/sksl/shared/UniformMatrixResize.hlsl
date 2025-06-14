cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float3x3 _12_testMatrix3x3 : packoffset(c0);
    float4 _12_colorGreen : packoffset(c3);
    float4 _12_colorRed : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float2x2 resizeMatrix_f22()
{
    return float2x2(_12_testMatrix3x3[0].xy, _12_testMatrix3x3[1].xy);
}

float4 main(float2 _41)
{
    float2x2 _45 = resizeMatrix_f22();
    float2 _53 = _45[0];
    float2 _56 = _45[1];
    bool _81 = false;
    if (all(bool2(_53.x == float2(1.0f, 2.0f).x, _53.y == float2(1.0f, 2.0f).y)) && all(bool2(_56.x == float2(4.0f, 5.0f).x, _56.y == float2(4.0f, 5.0f).y)))
    {
        float2x2 _62 = resizeMatrix_f22();
        float3 _64 = float3(_62[0], 0.0f);
        float3 _66 = float3(_62[1], 0.0f);
        _81 = (all(bool3(_64.x == float3(1.0f, 2.0f, 0.0f).x, _64.y == float3(1.0f, 2.0f, 0.0f).y, _64.z == float3(1.0f, 2.0f, 0.0f).z)) && all(bool3(_66.x == float3(4.0f, 5.0f, 0.0f).x, _66.y == float3(4.0f, 5.0f, 0.0f).y, _66.z == float3(4.0f, 5.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 1.0f).x == float3(0.0f, 0.0f, 1.0f).x, float3(0.0f, 0.0f, 1.0f).y == float3(0.0f, 0.0f, 1.0f).y, float3(0.0f, 0.0f, 1.0f).z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _81 = false;
    }
    float4 _82 = 0.0f.xxxx;
    if (_81)
    {
        _82 = _12_colorGreen;
    }
    else
    {
        _82 = _12_colorRed;
    }
    return _82;
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
