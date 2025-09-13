cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_src : packoffset(c0);
    float4 _11_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float4 _32 = _11_src + (_11_dst * (1.0f - _11_src.w));
    float4 _RESERVED_IDENTIFIER_FIXUP_0_result = _32;
    float3 _33 = max(_32.xyz, (_11_src.xyz * (1.0f - _11_dst.w)) + _11_dst.xyz);
    float4 _48 = _RESERVED_IDENTIFIER_FIXUP_0_result;
    float4 _49 = float4(_33.x, _33.y, _33.z, _48.w);
    _RESERVED_IDENTIFIER_FIXUP_0_result = _49;
    sk_FragColor = _49;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
