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
    bool _58 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _47 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
        float2 _50 = _47[0];
        float2 _54 = _47[1];
        _58 = all(bool2(_10_testMatrix2x2[0].x == _50.x, _10_testMatrix2x2[0].y == _50.y)) && all(bool2(_10_testMatrix2x2[1].x == _54.x, _10_testMatrix2x2[1].y == _54.y));
    }
    else
    {
        _58 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _58;
    bool _90 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _74 = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
        float3 _77 = _74[0];
        float3 _81 = _74[1];
        float3 _86 = _74[2];
        _90 = (all(bool3(_10_testMatrix3x3[0].x == _77.x, _10_testMatrix3x3[0].y == _77.y, _10_testMatrix3x3[0].z == _77.z)) && all(bool3(_10_testMatrix3x3[1].x == _81.x, _10_testMatrix3x3[1].y == _81.y, _10_testMatrix3x3[1].z == _81.z))) && all(bool3(_10_testMatrix3x3[2].x == _86.x, _10_testMatrix3x3[2].y == _86.y, _10_testMatrix3x3[2].z == _86.z));
    }
    else
    {
        _90 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _90;
    bool _109 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float2x2 _97 = float2x2(float2(100.0f, 0.0f), float2(0.0f, 100.0f));
        float2 _101 = _97[0];
        float2 _105 = _97[1];
        _109 = any(bool2(_10_testMatrix2x2[0].x != _101.x, _10_testMatrix2x2[0].y != _101.y)) || any(bool2(_10_testMatrix2x2[1].x != _105.x, _10_testMatrix2x2[1].y != _105.y));
    }
    else
    {
        _109 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _109;
    bool _133 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        float3x3 _118 = float3x3(float3(9.0f, 8.0f, 7.0f), float3(6.0f, 5.0f, 4.0f), float3(3.0f, 2.0f, 1.0f));
        float3 _120 = _118[0];
        float3 _124 = _118[1];
        float3 _129 = _118[2];
        _133 = (any(bool3(_10_testMatrix3x3[0].x != _120.x, _10_testMatrix3x3[0].y != _120.y, _10_testMatrix3x3[0].z != _120.z)) || any(bool3(_10_testMatrix3x3[1].x != _124.x, _10_testMatrix3x3[1].y != _124.y, _10_testMatrix3x3[1].z != _124.z))) || any(bool3(_10_testMatrix3x3[2].x != _129.x, _10_testMatrix3x3[2].y != _129.y, _10_testMatrix3x3[2].z != _129.z));
    }
    else
    {
        _133 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _133;
    float4 _135 = 0.0f.xxxx;
    if (_RESERVED_IDENTIFIER_FIXUP_0_ok)
    {
        _135 = _10_colorGreen;
    }
    else
    {
        _135 = _10_colorRed;
    }
    return _135;
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
