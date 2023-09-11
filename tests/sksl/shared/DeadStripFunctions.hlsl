cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _9_colorGreen : packoffset(c0);
    float4 _9_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 live_fn_h4h4h4(float4 _43, float4 _44)
{
    return _43 + _44;
}

float4 unpremul_h4h4(float4 _24)
{
    return float4(_24.xyz * (1.0f / max(_24.w, 9.9999997473787516355514526367188e-05f)), _24.w);
}

float4 main(float2 _50)
{
    float4 _56 = 3.0f.xxxx;
    float4 _59 = (-5.0f).xxxx;
    float4 _60 = live_fn_h4h4h4(_56, _59);
    float4 a = _60;
    float4 _62 = 1.0f.xxxx;
    float4 _63 = unpremul_h4h4(_62);
    float4 b = _63;
    bool _74 = false;
    if (any(bool4(_60.x != 0.0f.xxxx.x, _60.y != 0.0f.xxxx.y, _60.z != 0.0f.xxxx.z, _60.w != 0.0f.xxxx.w)))
    {
        _74 = any(bool4(_63.x != 0.0f.xxxx.x, _63.y != 0.0f.xxxx.y, _63.z != 0.0f.xxxx.z, _63.w != 0.0f.xxxx.w));
    }
    else
    {
        _74 = false;
    }
    float4 _75 = 0.0f.xxxx;
    if (_74)
    {
        _75 = _9_colorGreen;
    }
    else
    {
        _75 = _9_colorRed;
    }
    return _75;
}

void frag_main()
{
    float2 _19 = 0.0f.xx;
    sk_FragColor = main(_19);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
