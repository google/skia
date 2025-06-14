cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _20_src : packoffset(c0);
    float4 _20_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

float guarded_divide_Qhhh(float _25, float _26)
{
    return _25 / (_26 + _kGuardedDivideEpsilon);
}

float color_burn_component_Qhh2h2(float2 _36, float2 _37)
{
    float _45 = 0.0f;
    if (_37.y == _37.x)
    {
        _45 = _37.y;
    }
    else
    {
        _45 = 0.0f;
    }
    float dyTerm = _45;
    float _58 = 0.0f;
    if (abs(_36.x) >= 6.103515625e-05f)
    {
        float _75 = (_37.y - _37.x) * _36.y;
        float _78 = _36.x;
        _58 = _37.y - min(_37.y, guarded_divide_Qhhh(_75, _78));
    }
    else
    {
        _58 = _45;
    }
    float delta = _58;
    return ((_58 * _36.y) + (_36.x * (1.0f - _37.y))) + (_37.x * (1.0f - _36.y));
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _108 = _20_src.xw;
    float2 _113 = _20_dst.xw;
    float2 _118 = _20_src.yw;
    float2 _122 = _20_dst.yw;
    float2 _127 = _20_src.zw;
    float2 _131 = _20_dst.zw;
    sk_FragColor = float4(color_burn_component_Qhh2h2(_108, _113), color_burn_component_Qhh2h2(_118, _122), color_burn_component_Qhh2h2(_127, _131), _20_src.w + ((1.0f - _20_src.w) * _20_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
