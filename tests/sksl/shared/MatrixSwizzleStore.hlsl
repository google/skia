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

bool test4x4_b()
{
    float4 values = float4(4.0f, 3.0f, 2.0f, 1.0f);
    float4x4 _matrix = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    for (int index = 0; index < 4; index++)
    {
        _matrix[index] = float4(values.xw.y, _matrix[index].y, _matrix[index].z, values.xw.x);
        _matrix[index] = float4(_matrix[index].x, values.yz.y, values.yz.x, _matrix[index].w);
        values += 4.0f.xxxx;
    }
    return ((all(bool4(_matrix[0].x == _12_testMatrix4x4[0].x, _matrix[0].y == _12_testMatrix4x4[0].y, _matrix[0].z == _12_testMatrix4x4[0].z, _matrix[0].w == _12_testMatrix4x4[0].w)) && all(bool4(_matrix[1].x == _12_testMatrix4x4[1].x, _matrix[1].y == _12_testMatrix4x4[1].y, _matrix[1].z == _12_testMatrix4x4[1].z, _matrix[1].w == _12_testMatrix4x4[1].w))) && all(bool4(_matrix[2].x == _12_testMatrix4x4[2].x, _matrix[2].y == _12_testMatrix4x4[2].y, _matrix[2].z == _12_testMatrix4x4[2].z, _matrix[2].w == _12_testMatrix4x4[2].w))) && all(bool4(_matrix[3].x == _12_testMatrix4x4[3].x, _matrix[3].y == _12_testMatrix4x4[3].y, _matrix[3].z == _12_testMatrix4x4[3].z, _matrix[3].w == _12_testMatrix4x4[3].w));
}

float4 main(float2 _95)
{
    float3 _RESERVED_IDENTIFIER_FIXUP_1_values = float3(3.0f, 2.0f, 1.0f);
    float3x3 _RESERVED_IDENTIFIER_FIXUP_0_matrix = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    for (int _RESERVED_IDENTIFIER_FIXUP_2_index = 0; _RESERVED_IDENTIFIER_FIXUP_2_index < 3; _RESERVED_IDENTIFIER_FIXUP_2_index++)
    {
        _RESERVED_IDENTIFIER_FIXUP_0_matrix[_RESERVED_IDENTIFIER_FIXUP_2_index] = float3(_RESERVED_IDENTIFIER_FIXUP_1_values.xz.y, _RESERVED_IDENTIFIER_FIXUP_0_matrix[_RESERVED_IDENTIFIER_FIXUP_2_index].y, _RESERVED_IDENTIFIER_FIXUP_1_values.xz.x);
        _RESERVED_IDENTIFIER_FIXUP_0_matrix[_RESERVED_IDENTIFIER_FIXUP_2_index].y = _RESERVED_IDENTIFIER_FIXUP_1_values.y;
        _RESERVED_IDENTIFIER_FIXUP_1_values += 3.0f.xxx;
    }
    bool _151 = false;
    if ((all(bool3(_RESERVED_IDENTIFIER_FIXUP_0_matrix[0].x == _12_testMatrix3x3[0].x, _RESERVED_IDENTIFIER_FIXUP_0_matrix[0].y == _12_testMatrix3x3[0].y, _RESERVED_IDENTIFIER_FIXUP_0_matrix[0].z == _12_testMatrix3x3[0].z)) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_0_matrix[1].x == _12_testMatrix3x3[1].x, _RESERVED_IDENTIFIER_FIXUP_0_matrix[1].y == _12_testMatrix3x3[1].y, _RESERVED_IDENTIFIER_FIXUP_0_matrix[1].z == _12_testMatrix3x3[1].z))) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_0_matrix[2].x == _12_testMatrix3x3[2].x, _RESERVED_IDENTIFIER_FIXUP_0_matrix[2].y == _12_testMatrix3x3[2].y, _RESERVED_IDENTIFIER_FIXUP_0_matrix[2].z == _12_testMatrix3x3[2].z)))
    {
        _151 = test4x4_b();
    }
    else
    {
        _151 = false;
    }
    float4 _152 = 0.0f.xxxx;
    if (_151)
    {
        _152 = _12_colorGreen;
    }
    else
    {
        _152 = _12_colorRed;
    }
    return _152;
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
