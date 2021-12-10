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
    float4 _63 = live_fn_h4h4h4(_59, _62);
    float4 a = _63;
    float4 _65 = 1.0f.xxxx;
    float4 _66 = unpremul_h4h4(_65);
    float4 b = _66;
    bool _76 = false;
    if (any(bool4(_63.x != 0.0f.xxxx.x, _63.y != 0.0f.xxxx.y, _63.z != 0.0f.xxxx.z, _63.w != 0.0f.xxxx.w)))
    {
        _76 = any(bool4(_66.x != 0.0f.xxxx.x, _66.y != 0.0f.xxxx.y, _66.z != 0.0f.xxxx.z, _66.w != 0.0f.xxxx.w));
    }
    else
    {
        _76 = false;
    }
    float4 _77 = 0.0f.xxxx;
    if (_76)
    {
        _77 = _12_colorGreen;
    }
    else
    {
        _77 = _12_colorRed;
    }
    return _77;
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
