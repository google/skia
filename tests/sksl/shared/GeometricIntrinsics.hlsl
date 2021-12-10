cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float _RESERVED_IDENTIFIER_FIXUP_0_x = 1.0f;
    _RESERVED_IDENTIFIER_FIXUP_0_x = length(_RESERVED_IDENTIFIER_FIXUP_0_x);
    _RESERVED_IDENTIFIER_FIXUP_0_x = distance(_RESERVED_IDENTIFIER_FIXUP_0_x, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_0_x *= 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_0_x = sign(_RESERVED_IDENTIFIER_FIXUP_0_x);
    float2 _RESERVED_IDENTIFIER_FIXUP_1_x = float2(1.0f, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_1_x = length(_RESERVED_IDENTIFIER_FIXUP_1_x).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = distance(_RESERVED_IDENTIFIER_FIXUP_1_x, float2(3.0f, 4.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = dot(_RESERVED_IDENTIFIER_FIXUP_1_x, float2(3.0f, 4.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = normalize(_RESERVED_IDENTIFIER_FIXUP_1_x);
    return _10_colorGreen;
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
