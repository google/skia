cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 live_fn_h4h4h4(float4 _46, float4 _47)
{
    return _46 + _47;
}

float4 unpremul_h4h4(float4 _27)
{
    return float4(_27.xyz * (1.0f / max(_27.w, 9.9999997473787516355514526367188e-05f)), _27.w);
}

float4 main(float2 _53)
{
    float4 _59 = 3.0f.xxxx;
    float4 _62 = (-5.0f).xxxx;
    float4 a = live_fn_h4h4h4(_59, _62);
    float4 _65 = 1.0f.xxxx;
    float4 b = unpremul_h4h4(_65);
    bool _78 = false;
    if (any(bool4(a.x != 0.0f.xxxx.x, a.y != 0.0f.xxxx.y, a.z != 0.0f.xxxx.z, a.w != 0.0f.xxxx.w)))
    {
        _78 = any(bool4(b.x != 0.0f.xxxx.x, b.y != 0.0f.xxxx.y, b.z != 0.0f.xxxx.z, b.w != 0.0f.xxxx.w));
    }
    else
    {
        _78 = false;
    }
    float4 _79 = 0.0f.xxxx;
    if (_78)
    {
        _79 = _12_colorGreen;
    }
    else
    {
        _79 = _12_colorRed;
    }
    return _79;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
