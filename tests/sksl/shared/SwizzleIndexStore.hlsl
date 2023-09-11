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
    float3 expected = float3(3.0f, 2.0f, 1.0f);
    float3 vec = 0.0f.xxx;
    for (int c = 0; c < 3; c++)
    {
        for (int r = 0; r < 3; r++)
        {
            vec[int3(2, 1, 0)[r]] = _9_testMatrix3x3[c][r];
        }
        if (any(bool3(vec.x != expected.x, vec.y != expected.y, vec.z != expected.z)))
        {
            return false;
        }
        expected += 3.0f.xxx;
    }
    return true;
}

bool test4x4_b()
{
    float4 expected = float4(4.0f, 3.0f, 2.0f, 1.0f);
    float4 vec = 0.0f.xxxx;
    for (int c = 0; c < 4; c++)
    {
        for (int r = 0; r < 4; r++)
        {
            vec[int4(3, 2, 1, 0)[r]] = _9_testMatrix4x4[c][r];
        }
        if (any(bool4(vec.x != expected.x, vec.y != expected.y, vec.z != expected.z, vec.w != expected.w)))
        {
            return false;
        }
        expected += 4.0f.xxxx;
    }
    return true;
}

float4 main(float2 _138)
{
    bool _144 = false;
    if (test3x3_b())
    {
        _144 = test4x4_b();
    }
    else
    {
        _144 = false;
    }
    float4 _145 = 0.0f.xxxx;
    if (_144)
    {
        _145 = _9_colorGreen;
    }
    else
    {
        _145 = _9_colorRed;
    }
    return _145;
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
