cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c2);
    row_major float3x3 _10_testMatrix3x3 : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    bool _55 = false;
    if (true)
    {
        _55 = all(bool2(_10_testMatrix2x2[0].x == float2(1.0f, 2.0f).x, _10_testMatrix2x2[0].y == float2(1.0f, 2.0f).y)) && all(bool2(_10_testMatrix2x2[1].x == float2(3.0f, 4.0f).x, _10_testMatrix2x2[1].y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _55 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _55;
    bool _83 = false;
    if (_55)
    {
        _83 = (all(bool3(_10_testMatrix3x3[0].x == float3(1.0f, 2.0f, 3.0f).x, _10_testMatrix3x3[0].y == float3(1.0f, 2.0f, 3.0f).y, _10_testMatrix3x3[0].z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_10_testMatrix3x3[1].x == float3(4.0f, 5.0f, 6.0f).x, _10_testMatrix3x3[1].y == float3(4.0f, 5.0f, 6.0f).y, _10_testMatrix3x3[1].z == float3(4.0f, 5.0f, 6.0f).z))) && all(bool3(_10_testMatrix3x3[2].x == float3(7.0f, 8.0f, 9.0f).x, _10_testMatrix3x3[2].y == float3(7.0f, 8.0f, 9.0f).y, _10_testMatrix3x3[2].z == float3(7.0f, 8.0f, 9.0f).z));
    }
    else
    {
        _83 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _83;
    bool _99 = false;
    if (_83)
    {
        _99 = any(bool2(_10_testMatrix2x2[0].x != float2(100.0f, 0.0f).x, _10_testMatrix2x2[0].y != float2(100.0f, 0.0f).y)) || any(bool2(_10_testMatrix2x2[1].x != float2(0.0f, 100.0f).x, _10_testMatrix2x2[1].y != float2(0.0f, 100.0f).y));
    }
    else
    {
        _99 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _99;
    bool _119 = false;
    if (_99)
    {
        _119 = (any(bool3(_10_testMatrix3x3[0].x != float3(9.0f, 8.0f, 7.0f).x, _10_testMatrix3x3[0].y != float3(9.0f, 8.0f, 7.0f).y, _10_testMatrix3x3[0].z != float3(9.0f, 8.0f, 7.0f).z)) || any(bool3(_10_testMatrix3x3[1].x != float3(6.0f, 5.0f, 4.0f).x, _10_testMatrix3x3[1].y != float3(6.0f, 5.0f, 4.0f).y, _10_testMatrix3x3[1].z != float3(6.0f, 5.0f, 4.0f).z))) || any(bool3(_10_testMatrix3x3[2].x != float3(3.0f, 2.0f, 1.0f).x, _10_testMatrix3x3[2].y != float3(3.0f, 2.0f, 1.0f).y, _10_testMatrix3x3[2].z != float3(3.0f, 2.0f, 1.0f).z));
    }
    else
    {
        _119 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _119;
    float4 _120 = 0.0f.xxxx;
    if (_119)
    {
        _120 = _10_colorGreen;
    }
    else
    {
        _120 = _10_colorRed;
    }
    return _120;
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
