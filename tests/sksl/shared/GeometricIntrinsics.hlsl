cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float _RESERVED_IDENTIFIER_FIXUP_0_x = 1.0f;
    float _26 = length(1.0f);
    _RESERVED_IDENTIFIER_FIXUP_0_x = _26;
    float _27 = distance(_26, 2.0f);
    _RESERVED_IDENTIFIER_FIXUP_0_x = _27;
    float _29 = _27 * 2.0f;
    _RESERVED_IDENTIFIER_FIXUP_0_x = _29;
    _RESERVED_IDENTIFIER_FIXUP_0_x = sign(_29);
    float2 _RESERVED_IDENTIFIER_FIXUP_1_x = float2(1.0f, 2.0f);
    float2 _34 = length(float2(1.0f, 2.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = _34;
    float2 _39 = distance(_34, float2(3.0f, 4.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = _39;
    float2 _41 = dot(_39, float2(3.0f, 4.0f)).xx;
    _RESERVED_IDENTIFIER_FIXUP_1_x = _41;
    _RESERVED_IDENTIFIER_FIXUP_1_x = normalize(_41);
    return _7_colorGreen;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
