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
    float4 _RESERVED_IDENTIFIER_FIXUP_0_result = _29;
    float3 _30 = max(_29.xyz, (_7_src.xyz * (1.0f - _7_dst.w)) + _7_dst.xyz);
    float4 _45 = _RESERVED_IDENTIFIER_FIXUP_0_result;
    float4 _46 = float4(_30.x, _30.y, _30.z, _45.w);
    _RESERVED_IDENTIFIER_FIXUP_0_result = _46;
    sk_FragColor = _46;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
