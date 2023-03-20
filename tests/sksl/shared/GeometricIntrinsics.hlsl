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
    float _29 = length(1.0f);
    _RESERVED_IDENTIFIER_FIXUP_0_x = _29;
    float _30 = distance(_29, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_0_x = _30;
    float _32 = _30 * 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_0_x = _32;
    _RESERVED_IDENTIFIER_FIXUP_0_x = sign(_32);
    float2 _RESERVED_IDENTIFIER_FIXUP_1_x = float2(1.0f, 2.0f);
    float2 _37 = length(float2(1.0f, 2.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = _37;
    float2 _42 = distance(_37, float2(3.0f, 4.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = _42;
    float2 _44 = dot(_42, float2(3.0f, 4.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = _44;
    _RESERVED_IDENTIFIER_FIXUP_1_x = normalize(_44);
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
