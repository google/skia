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

float4 main(float2 _27)
{
    float3 _31 = cross(_11_testMatrix3x3[0], _11_testMatrix3x3[1]);
    bool _62 = false;
    if (all(bool3(_31.x == float3(-3.0f, 6.0f, -3.0f).x, _31.y == float3(-3.0f, 6.0f, -3.0f).y, _31.z == float3(-3.0f, 6.0f, -3.0f).z)))
    {
        float3 _50 = cross(_11_testMatrix3x3[2], _11_testMatrix3x3[0]);
        _62 = all(bool3(_50.x == float3(6.0f, -12.0f, 6.0f).x, _50.y == float3(6.0f, -12.0f, 6.0f).y, _50.z == float3(6.0f, -12.0f, 6.0f).z));
    }
    else
    {
        _62 = false;
    }
    float4 _63 = 0.0f.xxxx;
    if (_62)
    {
        _63 = _11_colorGreen;
    }
    else
    {
        _63 = _11_colorRed;
    }
    return _63;
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
