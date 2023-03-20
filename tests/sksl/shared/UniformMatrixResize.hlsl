cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float3x3 _11_testMatrix3x3 : packoffset(c0);
    float4 _11_colorGreen : packoffset(c3);
    float4 _11_colorRed : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float2x2 resizeMatrix_f22()
{
    return float2x2(_11_testMatrix3x3[0].xy, _11_testMatrix3x3[1].xy);
}

float4 main(float2 _41)
{
    float2x2 _44 = resizeMatrix_f22();
    float2 _52 = _44[0];
    float2 _55 = _44[1];
    bool _80 = false;
    if (all(bool2(_52.x == float2(1.0f, 2.0f).x, _52.y == float2(1.0f, 2.0f).y)) && all(bool2(_55.x == float2(4.0f, 5.0f).x, _55.y == float2(4.0f, 5.0f).y)))
    {
        float2x2 _61 = resizeMatrix_f22();
        float3 _63 = float3(_61[0], 0.0f);
        float3 _65 = float3(_61[1], 0.0f);
        _80 = (all(bool3(_63.x == float3(1.0f, 2.0f, 0.0f).x, _63.y == float3(1.0f, 2.0f, 0.0f).y, _63.z == float3(1.0f, 2.0f, 0.0f).z)) && all(bool3(_65.x == float3(4.0f, 5.0f, 0.0f).x, _65.y == float3(4.0f, 5.0f, 0.0f).y, _65.z == float3(4.0f, 5.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 1.0f).x == float3(0.0f, 0.0f, 1.0f).x, float3(0.0f, 0.0f, 1.0f).y == float3(0.0f, 0.0f, 1.0f).y, float3(0.0f, 0.0f, 1.0f).z == float3(0.0f, 0.0f, 1.0f).z));
    }
    else
    {
        _80 = false;
    }
    float4 _81 = 0.0f.xxxx;
    if (_80)
    {
        _81 = _11_colorGreen;
    }
    else
    {
        _81 = _11_colorRed;
    }
    return _81;
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
