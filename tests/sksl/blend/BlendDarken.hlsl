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
    float4 _RESERVED_IDENTIFIER_FIXUP_0_a = _32;
    float3 _47 = (_11_src.xyz * (1.0f - _11_dst.w)) + _11_dst.xyz;
    float3 _RESERVED_IDENTIFIER_FIXUP_1_b = _47;
    float3 _48 = min(_32.xyz, _47);
    float4 _50 = _RESERVED_IDENTIFIER_FIXUP_0_a;
    float4 _51 = float4(_48.x, _48.y, _48.z, _50.w);
    _RESERVED_IDENTIFIER_FIXUP_0_a = _51;
    sk_FragColor = _51;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
