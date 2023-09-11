cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_src : packoffset(c0);
    float4 _8_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float blend_overlay_component_Qhh2h2(float2 _14, float2 _15)
{
    float _25 = 0.0f;
    if ((2.0f * _15.x) <= _15.y)
    {
        _25 = (2.0f * _14.x) * _15.x;
    }
    else
    {
        _25 = (_14.y * _15.y) - ((2.0f * (_15.y - _15.x)) * (_14.y - _14.x));
    }
    return _25;
}

void frag_main()
{
    float2 _66 = _8_src.xw;
    float2 _71 = _8_dst.xw;
    float2 _76 = _8_src.yw;
    float2 _80 = _8_dst.yw;
    float2 _85 = _8_src.zw;
    float2 _89 = _8_dst.zw;
    float4 _RESERVED_IDENTIFIER_FIXUP_0_result = float4(blend_overlay_component_Qhh2h2(_66, _71), blend_overlay_component_Qhh2h2(_76, _80), blend_overlay_component_Qhh2h2(_85, _89), _8_src.w + ((1.0f - _8_src.w) * _8_dst.w));
    float4 _105 = _RESERVED_IDENTIFIER_FIXUP_0_result;
    float3 _125 = _105.xyz + ((_8_dst.xyz * (1.0f - _8_src.w)) + (_8_src.xyz * (1.0f - _8_dst.w)));
    float4 _126 = _RESERVED_IDENTIFIER_FIXUP_0_result;
    float4 _127 = float4(_125.x, _125.y, _125.z, _126.w);
    _RESERVED_IDENTIFIER_FIXUP_0_result = _127;
    sk_FragColor = _127;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
