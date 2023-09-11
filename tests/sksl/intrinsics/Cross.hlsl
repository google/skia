cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float3x3 _7_testMatrix3x3 : packoffset(c0);
    float4 _7_colorGreen : packoffset(c3);
    float4 _7_colorRed : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _23)
{
    float3 _27 = cross(_7_testMatrix3x3[0], _7_testMatrix3x3[1]);
    bool _59 = false;
    if (all(bool3(_27.x == float3(-3.0f, 6.0f, -3.0f).x, _27.y == float3(-3.0f, 6.0f, -3.0f).y, _27.z == float3(-3.0f, 6.0f, -3.0f).z)))
    {
        float3 _47 = cross(_7_testMatrix3x3[2], _7_testMatrix3x3[0]);
        _59 = all(bool3(_47.x == float3(6.0f, -12.0f, 6.0f).x, _47.y == float3(6.0f, -12.0f, 6.0f).y, _47.z == float3(6.0f, -12.0f, 6.0f).z));
    }
    else
    {
        _59 = false;
    }
    float4 _60 = 0.0f.xxxx;
    if (_59)
    {
        _60 = _7_colorGreen;
    }
    else
    {
        _60 = _7_colorRed;
    }
    return _60;
}

void frag_main()
{
    float2 _19 = 0.0f.xx;
    sk_FragColor = main(_19);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
