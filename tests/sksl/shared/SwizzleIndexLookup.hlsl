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
    float3 expected = float3(3.0f, 2.0f, 1.0f);
    for (int c = 0; c < 3; c++)
    {
        float3 vec = _12_testMatrix3x3[c];
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
        float4 vec = _12_testMatrix4x4[c];
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

float4 main(float2 _131)
{
    bool _137 = false;
    if (test3x3_b())
    {
        _137 = test4x4_b();
    }
    else
    {
        _137 = false;
    }
    float4 _138 = 0.0f.xxxx;
    if (_137)
    {
        _138 = _12_colorGreen;
    }
    else
    {
        _138 = _12_colorRed;
    }
    return _138;
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
