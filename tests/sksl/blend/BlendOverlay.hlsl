cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_src : packoffset(c0);
    float4 _12_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float blend_overlay_component_Qhh2h2(float2 _18, float2 _19)
{
    float _29 = 0.0f;
    if ((2.0f * _19.x) <= _19.y)
    {
        _29 = (2.0f * _18.x) * _19.x;
    }
    else
    {
        _29 = (_18.y * _19.y) - ((2.0f * (_19.y - _19.x)) * (_18.y - _18.x));
    }
    return _29;
}

void frag_main()
{
    float2 _69 = _12_src.xw;
    float2 _74 = _12_dst.xw;
    float2 _79 = _12_src.yw;
    float2 _83 = _12_dst.yw;
    float2 _88 = _12_src.zw;
    float2 _92 = _12_dst.zw;
    float4 _RESERVED_IDENTIFIER_FIXUP_0_result = float4(blend_overlay_component_Qhh2h2(_69, _74), blend_overlay_component_Qhh2h2(_79, _83), blend_overlay_component_Qhh2h2(_88, _92), _12_src.w + ((1.0f - _12_src.w) * _12_dst.w));
    float4 _108 = _RESERVED_IDENTIFIER_FIXUP_0_result;
    float3 _128 = _108.xyz + ((_12_dst.xyz * (1.0f - _12_src.w)) + (_12_src.xyz * (1.0f - _12_dst.w)));
    float4 _129 = _RESERVED_IDENTIFIER_FIXUP_0_result;
    float4 _130 = float4(_128.x, _128.y, _128.z, _129.w);
    _RESERVED_IDENTIFIER_FIXUP_0_result = _130;
    sk_FragColor = _130;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
