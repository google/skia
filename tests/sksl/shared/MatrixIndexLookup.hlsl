cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
    row_major float3x3 _12_testMatrix3x3 : packoffset(c2);
    row_major float4x4 _12_testMatrix4x4 : packoffset(c5);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test3x3_b()
{
    float3x3 _matrix = _12_testMatrix3x3;
    float3 expected = float3(1.0f, 2.0f, 3.0f);
    for (int index = 0; index < 3; index++)
    {
        if (any(bool3(_matrix[index].x != expected.x, _matrix[index].y != expected.y, _matrix[index].z != expected.z)))
        {
            return false;
        }
        expected += 3.0f.xxx;
    }
    return true;
}

bool test4x4_b()
{
    float4x4 _matrix = _12_testMatrix4x4;
    float4 expected = float4(1.0f, 2.0f, 3.0f, 4.0f);
    for (int index = 0; index < 4; index++)
    {
        if (any(bool4(_matrix[index].x != expected.x, _matrix[index].y != expected.y, _matrix[index].z != expected.z, _matrix[index].w != expected.w)))
        {
            return false;
        }
        expected += 4.0f.xxxx;
    }
    return true;
}

float4 main(float2 _105)
{
    bool _111 = false;
    if (test3x3_b())
    {
        _111 = test4x4_b();
    }
    else
    {
        _111 = false;
    }
    float4 _112 = 0.0f.xxxx;
    if (_111)
    {
        _112 = _12_colorGreen;
    }
    else
    {
        _112 = _12_colorRed;
    }
    return _112;
}

void frag_main()
{
    float2 _25 = 0.0f.xx;
    sk_FragColor = main(_25);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
