cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorGreen : packoffset(c0);
    float4 _8_colorRed : packoffset(c1);
    row_major float3x3 _8_testMatrix3x3 : packoffset(c2);
    row_major float4x4 _8_testMatrix4x4 : packoffset(c5);
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
    return ((all(bool4(_matrix[0].x == _8_testMatrix4x4[0].x, _matrix[0].y == _8_testMatrix4x4[0].y, _matrix[0].z == _8_testMatrix4x4[0].z, _matrix[0].w == _8_testMatrix4x4[0].w)) && all(bool4(_matrix[1].x == _8_testMatrix4x4[1].x, _matrix[1].y == _8_testMatrix4x4[1].y, _matrix[1].z == _8_testMatrix4x4[1].z, _matrix[1].w == _8_testMatrix4x4[1].w))) && all(bool4(_matrix[2].x == _8_testMatrix4x4[2].x, _matrix[2].y == _8_testMatrix4x4[2].y, _matrix[2].z == _8_testMatrix4x4[2].z, _matrix[2].w == _8_testMatrix4x4[2].w))) && all(bool4(_matrix[3].x == _8_testMatrix4x4[3].x, _matrix[3].y == _8_testMatrix4x4[3].y, _matrix[3].z == _8_testMatrix4x4[3].z, _matrix[3].w == _8_testMatrix4x4[3].w));
}

float4 main(float2 _92)
{
    float3 _RESERVED_IDENTIFIER_FIXUP_1_values = float3(3.0f, 2.0f, 1.0f);
    float3x3 _RESERVED_IDENTIFIER_FIXUP_0_matrix = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    for (int _RESERVED_IDENTIFIER_FIXUP_2_index = 0; _RESERVED_IDENTIFIER_FIXUP_2_index < 3; _RESERVED_IDENTIFIER_FIXUP_2_index++)
    {
        _RESERVED_IDENTIFIER_FIXUP_0_matrix[_RESERVED_IDENTIFIER_FIXUP_2_index] = float3(_RESERVED_IDENTIFIER_FIXUP_1_values.xz.y, _RESERVED_IDENTIFIER_FIXUP_0_matrix[_RESERVED_IDENTIFIER_FIXUP_2_index].y, _RESERVED_IDENTIFIER_FIXUP_1_values.xz.x);
        _RESERVED_IDENTIFIER_FIXUP_0_matrix[_RESERVED_IDENTIFIER_FIXUP_2_index].y = _RESERVED_IDENTIFIER_FIXUP_1_values.y;
        _RESERVED_IDENTIFIER_FIXUP_1_values += 3.0f.xxx;
    }
    bool _148 = false;
    if ((all(bool3(_RESERVED_IDENTIFIER_FIXUP_0_matrix[0].x == _8_testMatrix3x3[0].x, _RESERVED_IDENTIFIER_FIXUP_0_matrix[0].y == _8_testMatrix3x3[0].y, _RESERVED_IDENTIFIER_FIXUP_0_matrix[0].z == _8_testMatrix3x3[0].z)) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_0_matrix[1].x == _8_testMatrix3x3[1].x, _RESERVED_IDENTIFIER_FIXUP_0_matrix[1].y == _8_testMatrix3x3[1].y, _RESERVED_IDENTIFIER_FIXUP_0_matrix[1].z == _8_testMatrix3x3[1].z))) && all(bool3(_RESERVED_IDENTIFIER_FIXUP_0_matrix[2].x == _8_testMatrix3x3[2].x, _RESERVED_IDENTIFIER_FIXUP_0_matrix[2].y == _8_testMatrix3x3[2].y, _RESERVED_IDENTIFIER_FIXUP_0_matrix[2].z == _8_testMatrix3x3[2].z)))
    {
        _148 = test4x4_b();
    }
    else
    {
        _148 = false;
    }
    float4 _149 = 0.0f.xxxx;
    if (_148)
    {
        _149 = _8_colorGreen;
    }
    else
    {
        _149 = _8_colorRed;
    }
    return _149;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
