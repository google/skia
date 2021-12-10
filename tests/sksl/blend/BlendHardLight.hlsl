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
    float _28 = 0.0f;
    if ((2.0f * _19.x) <= _19.y)
    {
        _28 = (2.0f * _18.x) * _19.x;
    }
    else
    {
        _28 = (_18.y * _19.y) - ((2.0f * (_19.y - _19.x)) * (_18.y - _18.x));
    }
    return _28;
}

float4 blend_overlay_h4h4h4(float4 _60, float4 _61)
{
    float2 _66 = _60.xw;
    float2 _69 = _61.xw;
    float2 _73 = _60.yw;
    float2 _76 = _61.yw;
    float2 _80 = _60.zw;
    float2 _83 = _61.zw;
    float4 result = float4(blend_overlay_component_Qhh2h2(_66, _69), blend_overlay_component_Qhh2h2(_73, _76), blend_overlay_component_Qhh2h2(_80, _83), _60.w + ((1.0f - _60.w) * _61.w));
    float4 _96 = result;
    float3 _112 = _96.xyz + ((_61.xyz * (1.0f - _60.w)) + (_60.xyz * (1.0f - _61.w)));
    float4 _113 = result;
    float4 _114 = float4(_112.x, _112.y, _112.z, _113.w);
    result = _114;
    return _114;
}

void frag_main()
{
    float4 _123 = _12_dst;
    float4 _127 = _12_src;
    sk_FragColor = blend_overlay_h4h4h4(_123, _127);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
