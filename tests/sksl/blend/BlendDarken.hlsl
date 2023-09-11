cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_src : packoffset(c0);
    float4 _7_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float4 _29 = _7_src + (_7_dst * (1.0f - _7_src.w));
    float4 _RESERVED_IDENTIFIER_FIXUP_0_a = _29;
    float3 _44 = (_7_src.xyz * (1.0f - _7_dst.w)) + _7_dst.xyz;
    float3 _RESERVED_IDENTIFIER_FIXUP_1_b = _44;
    float3 _45 = min(_29.xyz, _44);
    float4 _47 = _RESERVED_IDENTIFIER_FIXUP_0_a;
    float4 _48 = float4(_45.x, _45.y, _45.z, _47.w);
    _RESERVED_IDENTIFIER_FIXUP_0_a = _48;
    sk_FragColor = _48;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
