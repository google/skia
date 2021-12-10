cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_src : packoffset(c0);
    float4 _10_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float4 _RESERVED_IDENTIFIER_FIXUP_0_result = _10_src + (_10_dst * (1.0f - _10_src.w));
    float3 _33 = max(_RESERVED_IDENTIFIER_FIXUP_0_result.xyz, (_10_src.xyz * (1.0f - _10_dst.w)) + _10_dst.xyz);
    _RESERVED_IDENTIFIER_FIXUP_0_result = float4(_33.x, _33.y, _33.z, _RESERVED_IDENTIFIER_FIXUP_0_result.w);
    sk_FragColor = _RESERVED_IDENTIFIER_FIXUP_0_result;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
