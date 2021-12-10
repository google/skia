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
    float2x2 _43 = resizeMatrix_f22();
    float2 _51 = _43[0];
    float2 _54 = _43[1];
    float4 _58 = 0.0f.xxxx;
    if (all(bool2(_51.x == float2(1.0f, 2.0f).x, _51.y == float2(1.0f, 2.0f).y)) && all(bool2(_54.x == float2(4.0f, 5.0f).x, _54.y == float2(4.0f, 5.0f).y)))
    {
        _58 = _11_colorGreen;
    }
    else
    {
        _58 = _11_colorRed;
    }
    return _58;
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
