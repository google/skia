cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 _RESERVED_IDENTIFIER_FIXUP_0_v = _10_testInputs;
    float _RESERVED_IDENTIFIER_FIXUP_1_x = _RESERVED_IDENTIFIER_FIXUP_0_v.x;
    float _RESERVED_IDENTIFIER_FIXUP_2_y = _RESERVED_IDENTIFIER_FIXUP_0_v.y;
    float _RESERVED_IDENTIFIER_FIXUP_3_z = _RESERVED_IDENTIFIER_FIXUP_0_v.z;
    float _RESERVED_IDENTIFIER_FIXUP_4_w = _RESERVED_IDENTIFIER_FIXUP_0_v.w;
    float4 a = float4(_RESERVED_IDENTIFIER_FIXUP_1_x, _RESERVED_IDENTIFIER_FIXUP_2_y, _RESERVED_IDENTIFIER_FIXUP_3_z, _RESERVED_IDENTIFIER_FIXUP_4_w);
    float _RESERVED_IDENTIFIER_FIXUP_9_x = _10_testInputs.x;
    float _RESERVED_IDENTIFIER_FIXUP_10_y = _10_testInputs.y;
    float _RESERVED_IDENTIFIER_FIXUP_11_z = _10_testInputs.z;
    float _RESERVED_IDENTIFIER_FIXUP_12_w = _10_testInputs.w;
    float4 b = float4(_RESERVED_IDENTIFIER_FIXUP_9_x, _RESERVED_IDENTIFIER_FIXUP_10_y, _RESERVED_IDENTIFIER_FIXUP_11_z, _RESERVED_IDENTIFIER_FIXUP_12_w);
    float4 _RESERVED_IDENTIFIER_FIXUP_13_v = float4(0.0f, 1.0f, 2.0f, 3.0f);
    float _RESERVED_IDENTIFIER_FIXUP_14_x = _RESERVED_IDENTIFIER_FIXUP_13_v.x;
    float _RESERVED_IDENTIFIER_FIXUP_15_y = _RESERVED_IDENTIFIER_FIXUP_13_v.y;
    float _RESERVED_IDENTIFIER_FIXUP_16_z = _RESERVED_IDENTIFIER_FIXUP_13_v.z;
    float _RESERVED_IDENTIFIER_FIXUP_17_w = _RESERVED_IDENTIFIER_FIXUP_13_v.w;
    float4 c = float4(_RESERVED_IDENTIFIER_FIXUP_14_x, _RESERVED_IDENTIFIER_FIXUP_15_y, _RESERVED_IDENTIFIER_FIXUP_16_z, _RESERVED_IDENTIFIER_FIXUP_17_w);
    bool _111 = false;
    if (all(bool4(a.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, a.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, a.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, a.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w)))
    {
        _111 = all(bool4(b.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, b.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, b.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, b.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w));
    }
    else
    {
        _111 = false;
    }
    bool _117 = false;
    if (_111)
    {
        _117 = all(bool4(c.x == float4(0.0f, 1.0f, 2.0f, 3.0f).x, c.y == float4(0.0f, 1.0f, 2.0f, 3.0f).y, c.z == float4(0.0f, 1.0f, 2.0f, 3.0f).z, c.w == float4(0.0f, 1.0f, 2.0f, 3.0f).w));
    }
    else
    {
        _117 = false;
    }
    float4 _118 = 0.0f.xxxx;
    if (_117)
    {
        _118 = _10_colorGreen;
    }
    else
    {
        _118 = _10_colorRed;
    }
    return _118;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
