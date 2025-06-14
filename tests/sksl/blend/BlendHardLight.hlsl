cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _13_src : packoffset(c0);
    float4 _13_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float blend_overlay_component_Qhh2h2(float2 _19, float2 _20)
{
    float _30 = 0.0f;
    if ((2.0f * _20.x) <= _20.y)
    {
        _30 = (2.0f * _19.x) * _20.x;
    }
    else
    {
        _30 = (_19.y * _20.y) - ((2.0f * (_20.y - _20.x)) * (_19.y - _19.x));
    }
    return _30;
}

float4 blend_overlay_h4h4h4(float4 _62, float4 _63)
{
    float2 _68 = _62.xw;
    float2 _71 = _63.xw;
    float2 _75 = _62.yw;
    float2 _78 = _63.yw;
    float2 _82 = _62.zw;
    float2 _85 = _63.zw;
    float4 result = float4(blend_overlay_component_Qhh2h2(_68, _71), blend_overlay_component_Qhh2h2(_75, _78), blend_overlay_component_Qhh2h2(_82, _85), _62.w + ((1.0f - _62.w) * _63.w));
    float4 _98 = result;
    float3 _114 = _98.xyz + ((_63.xyz * (1.0f - _62.w)) + (_62.xyz * (1.0f - _63.w)));
    float4 _115 = result;
    float4 _116 = float4(_114.x, _114.y, _114.z, _115.w);
    result = _116;
    return _116;
}

void frag_main()
{
    float4 _124 = _13_dst;
    float4 _128 = _13_src;
    sk_FragColor = blend_overlay_h4h4h4(_124, _128);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
