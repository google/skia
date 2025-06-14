cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _13_colorGreen : packoffset(c0);
    float4 _13_colorRed : packoffset(c1);
    row_major float3x3 _13_testMatrix3x3 : packoffset(c2);
    row_major float4x4 _13_testMatrix4x4 : packoffset(c5);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test3x3_b()
{
    float3 expected = float3(3.0f, 2.0f, 1.0f);
    for (int c = 0; c < 3; c++)
    {
        float3 vec = _13_testMatrix3x3[c];
        for (int r = 0; r < 3; r++)
        {
            if (vec.zyx[r] != expected[r])
            {
                return false;
            }
        }
        expected += 3.0f.xxx;
    }
    return true;
}

bool test4x4_b()
{
    float4 expected = float4(4.0f, 3.0f, 2.0f, 1.0f);
    for (int c = 0; c < 4; c++)
    {
        float4 vec = _13_testMatrix4x4[c];
        for (int r = 0; r < 4; r++)
        {
            if (vec.wzyx[r] != expected[r])
            {
                return false;
            }
        }
        expected += 4.0f.xxxx;
    }
    return true;
}

float4 main(float2 _132)
{
    bool _138 = false;
    if (test3x3_b())
    {
        _138 = test4x4_b();
    }
    else
    {
        _138 = false;
    }
    float4 _139 = 0.0f.xxxx;
    if (_138)
    {
        _139 = _13_colorGreen;
    }
    else
    {
        _139 = _13_colorRed;
    }
    return _139;
}

void frag_main()
{
    float2 _26 = 0.0f.xx;
    sk_FragColor = main(_26);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
