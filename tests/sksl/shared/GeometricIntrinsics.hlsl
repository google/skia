cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float _RESERVED_IDENTIFIER_FIXUP_0_x = 1.0f;
    float _30 = length(1.0f);
    _RESERVED_IDENTIFIER_FIXUP_0_x = _30;
    float _31 = distance(_30, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_0_x = _31;
    float _33 = _31 * 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_0_x = _33;
    _RESERVED_IDENTIFIER_FIXUP_0_x = sign(_33);
    float2 _RESERVED_IDENTIFIER_FIXUP_1_x = float2(1.0f, 2.0f);
    float2 _38 = length(float2(1.0f, 2.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = _38;
    float2 _43 = distance(_38, float2(3.0f, 4.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = _43;
    float2 _45 = dot(_43, float2(3.0f, 4.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = _45;
    _RESERVED_IDENTIFIER_FIXUP_1_x = normalize(_45);
    return _11_colorGreen;
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
