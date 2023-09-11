cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _9_src : packoffset(c0);
    float4 _9_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float blend_overlay_component_Qhh2h2(float2 _15, float2 _16)
{
    float _26 = 0.0f;
    if ((2.0f * _16.x) <= _16.y)
    {
        _26 = (2.0f * _15.x) * _16.x;
    }
    else
    {
        _26 = (_15.y * _16.y) - ((2.0f * (_16.y - _16.x)) * (_15.y - _15.x));
    }
    return _26;
}

float4 blend_overlay_h4h4h4(float4 _58, float4 _59)
{
    float2 _64 = _58.xw;
    float2 _67 = _59.xw;
    float2 _71 = _58.yw;
    float2 _74 = _59.yw;
    float2 _78 = _58.zw;
    float2 _81 = _59.zw;
    float4 result = float4(blend_overlay_component_Qhh2h2(_64, _67), blend_overlay_component_Qhh2h2(_71, _74), blend_overlay_component_Qhh2h2(_78, _81), _58.w + ((1.0f - _58.w) * _59.w));
    float4 _94 = result;
    float3 _110 = _94.xyz + ((_59.xyz * (1.0f - _58.w)) + (_58.xyz * (1.0f - _59.w)));
    float4 _111 = result;
    float4 _112 = float4(_110.x, _110.y, _110.z, _111.w);
    result = _112;
    return _112;
}

void frag_main()
{
    float4 _121 = _9_dst;
    float4 _125 = _9_src;
    sk_FragColor = blend_overlay_h4h4h4(_121, _125);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
