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
    float3 values = float3(1.0f, 2.0f, 3.0f);
    float3x3 _matrix = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    for (int index = 0; index < 3; index++)
    {
        _matrix[index] = values;
        values += 3.0f.xxx;
    }
    return (all(bool3(_matrix[0].x == _12_testMatrix3x3[0].x, _matrix[0].y == _12_testMatrix3x3[0].y, _matrix[0].z == _12_testMatrix3x3[0].z)) && all(bool3(_matrix[1].x == _12_testMatrix3x3[1].x, _matrix[1].y == _12_testMatrix3x3[1].y, _matrix[1].z == _12_testMatrix3x3[1].z))) && all(bool3(_matrix[2].x == _12_testMatrix3x3[2].x, _matrix[2].y == _12_testMatrix3x3[2].y, _matrix[2].z == _12_testMatrix3x3[2].z));
}

bool test4x4_b()
{
    float4 values = float4(1.0f, 2.0f, 3.0f, 4.0f);
    float4x4 _matrix = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    for (int index = 0; index < 4; index++)
    {
        _matrix[index] = values;
        values += 4.0f.xxxx;
    }
    return ((all(bool4(_matrix[0].x == _12_testMatrix4x4[0].x, _matrix[0].y == _12_testMatrix4x4[0].y, _matrix[0].z == _12_testMatrix4x4[0].z, _matrix[0].w == _12_testMatrix4x4[0].w)) && all(bool4(_matrix[1].x == _12_testMatrix4x4[1].x, _matrix[1].y == _12_testMatrix4x4[1].y, _matrix[1].z == _12_testMatrix4x4[1].z, _matrix[1].w == _12_testMatrix4x4[1].w))) && all(bool4(_matrix[2].x == _12_testMatrix4x4[2].x, _matrix[2].y == _12_testMatrix4x4[2].y, _matrix[2].z == _12_testMatrix4x4[2].z, _matrix[2].w == _12_testMatrix4x4[2].w))) && all(bool4(_matrix[3].x == _12_testMatrix4x4[3].x, _matrix[3].y == _12_testMatrix4x4[3].y, _matrix[3].z == _12_testMatrix4x4[3].z, _matrix[3].w == _12_testMatrix4x4[3].w));
}

float4 main(float2 _128)
{
    bool _135 = false;
    if (test3x3_b())
    {
        _135 = test4x4_b();
    }
    else
    {
        _135 = false;
    }
    float4 _136 = 0.0f.xxxx;
    if (_135)
    {
        _136 = _12_colorGreen;
    }
    else
    {
        _136 = _12_colorRed;
    }
    return _136;
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
