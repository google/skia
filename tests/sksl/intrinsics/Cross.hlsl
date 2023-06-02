cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float3x3 _10_testMatrix3x3 : packoffset(c0);
    float4 _10_colorGreen : packoffset(c3);
    float4 _10_colorRed : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    float3 _29 = cross(_10_testMatrix3x3[0], _10_testMatrix3x3[1]);
    bool _61 = false;
    if (all(bool3(_29.x == float3(-3.0f, 6.0f, -3.0f).x, _29.y == float3(-3.0f, 6.0f, -3.0f).y, _29.z == float3(-3.0f, 6.0f, -3.0f).z)))
    {
        float3 _49 = cross(_10_testMatrix3x3[2], _10_testMatrix3x3[0]);
        _61 = all(bool3(_49.x == float3(6.0f, -12.0f, 6.0f).x, _49.y == float3(6.0f, -12.0f, 6.0f).y, _49.z == float3(6.0f, -12.0f, 6.0f).z));
    }
    else
    {
        _61 = false;
    }
    bool4 _63 = _61.xxxx;
    return float4(_63.x ? _10_colorGreen.x : _10_colorRed.x, _63.y ? _10_colorGreen.y : _10_colorRed.y, _63.z ? _10_colorGreen.z : _10_colorRed.z, _63.w ? _10_colorGreen.w : _10_colorRed.w);
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
