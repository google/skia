cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _13_colorGreen : packoffset(c0);
    float4 _13_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 live_fn_h4h4h4(float4 _47, float4 _48)
{
    return _47 + _48;
}

float4 unpremul_h4h4(float4 _28)
{
    return float4(_28.xyz * (1.0f / max(_28.w, 9.9999997473787516355514526367188e-05f)), _28.w);
}

float4 main(float2 _54)
{
    float4 _60 = 3.0f.xxxx;
    float4 _63 = (-5.0f).xxxx;
    float4 _64 = live_fn_h4h4h4(_60, _63);
    float4 a = _64;
    float4 _66 = 1.0f.xxxx;
    float4 _67 = unpremul_h4h4(_66);
    float4 b = _67;
    bool _78 = false;
    if (any(bool4(_64.x != 0.0f.xxxx.x, _64.y != 0.0f.xxxx.y, _64.z != 0.0f.xxxx.z, _64.w != 0.0f.xxxx.w)))
    {
        _78 = any(bool4(_67.x != 0.0f.xxxx.x, _67.y != 0.0f.xxxx.y, _67.z != 0.0f.xxxx.z, _67.w != 0.0f.xxxx.w));
    }
    else
    {
        _78 = false;
    }
    float4 _79 = 0.0f.xxxx;
    if (_78)
    {
        _79 = _13_colorGreen;
    }
    else
    {
        _79 = _13_colorRed;
    }
    return _79;
}

void frag_main()
{
    float2 _23 = 0.0f.xx;
    sk_FragColor = main(_23);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
