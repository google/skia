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

float _blend_overlay_component_hh2h2(float2 _17, float2 _18)
{
    float _27 = 0.0f;
    if ((2.0f * _18.x) <= _18.y)
    {
        _27 = (2.0f * _17.x) * _18.x;
    }
    else
    {
        _27 = (_17.y * _18.y) - ((2.0f * (_18.y - _18.x)) * (_17.y - _17.x));
    }
    return _27;
}

void frag_main()
{
    float2 _68 = _11_src.xw;
    float2 _73 = _11_dst.xw;
    float2 _78 = _11_src.yw;
    float2 _82 = _11_dst.yw;
    float2 _87 = _11_src.zw;
    float2 _91 = _11_dst.zw;
    float4 _RESERVED_IDENTIFIER_FIXUP_0_result = float4(_blend_overlay_component_hh2h2(_68, _73), _blend_overlay_component_hh2h2(_78, _82), _blend_overlay_component_hh2h2(_87, _91), _11_src.w + ((1.0f - _11_src.w) * _11_dst.w));
    float3 _127 = _RESERVED_IDENTIFIER_FIXUP_0_result.xyz + ((_11_dst.xyz * (1.0f - _11_src.w)) + (_11_src.xyz * (1.0f - _11_dst.w)));
    _RESERVED_IDENTIFIER_FIXUP_0_result = float4(_127.x, _127.y, _127.z, _RESERVED_IDENTIFIER_FIXUP_0_result.w);
    sk_FragColor = _RESERVED_IDENTIFIER_FIXUP_0_result;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
