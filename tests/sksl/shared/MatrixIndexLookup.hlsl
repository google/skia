cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _9_colorGreen : packoffset(c0);
    float4 _9_colorRed : packoffset(c1);
    row_major float3x3 _9_testMatrix3x3 : packoffset(c2);
    row_major float4x4 _9_testMatrix4x4 : packoffset(c5);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test3x3_b()
{
    float3x3 _matrix = _9_testMatrix3x3;
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
    float4x4 _matrix = _9_testMatrix4x4;
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

float4 main(float2 _103)
{
    bool _109 = false;
    if (test3x3_b())
    {
        _109 = test4x4_b();
    }
    else
    {
        _109 = false;
    }
    float4 _110 = 0.0f.xxxx;
    if (_109)
    {
        _110 = _9_colorGreen;
    }
    else
    {
        _110 = _9_colorRed;
    }
    return _110;
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
