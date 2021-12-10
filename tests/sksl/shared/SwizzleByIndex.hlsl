cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorBlack : packoffset(c1);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 _RESERVED_IDENTIFIER_FIXUP_0_v = _10_testInputs;
    int4 _RESERVED_IDENTIFIER_FIXUP_1_i = int4(int(_10_colorBlack.x), int(_10_colorBlack.y), int(_10_colorBlack.z), int(_10_colorBlack.w));
    float _RESERVED_IDENTIFIER_FIXUP_2_x = _RESERVED_IDENTIFIER_FIXUP_0_v[_RESERVED_IDENTIFIER_FIXUP_1_i.x];
    float _RESERVED_IDENTIFIER_FIXUP_3_y = _RESERVED_IDENTIFIER_FIXUP_0_v[_RESERVED_IDENTIFIER_FIXUP_1_i.y];
    float _RESERVED_IDENTIFIER_FIXUP_4_z = _RESERVED_IDENTIFIER_FIXUP_0_v[_RESERVED_IDENTIFIER_FIXUP_1_i.z];
    float _RESERVED_IDENTIFIER_FIXUP_5_w = _RESERVED_IDENTIFIER_FIXUP_0_v[_RESERVED_IDENTIFIER_FIXUP_1_i.w];
    float4 _73 = float4(_RESERVED_IDENTIFIER_FIXUP_2_x, _RESERVED_IDENTIFIER_FIXUP_3_y, _RESERVED_IDENTIFIER_FIXUP_4_z, _RESERVED_IDENTIFIER_FIXUP_5_w);
    float4 _79 = 0.0f.xxxx;
    if (all(bool4(_73.x == float4(-1.25f, -1.25f, -1.25f, 0.0f).x, _73.y == float4(-1.25f, -1.25f, -1.25f, 0.0f).y, _73.z == float4(-1.25f, -1.25f, -1.25f, 0.0f).z, _73.w == float4(-1.25f, -1.25f, -1.25f, 0.0f).w)))
    {
        _79 = _10_colorGreen;
    }
    else
    {
        _79 = _10_colorRed;
    }
    return _79;
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
