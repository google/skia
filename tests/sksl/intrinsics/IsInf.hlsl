cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _10_testMatrix2x2 : packoffset(c0);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 infiniteValue = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * (1.0f / _10_colorGreen.x);
    float4 finiteValue = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * (1.0f / _10_colorGreen.y);
    bool _71 = false;
    if (isinf(infiniteValue.x))
    {
        _71 = all(isinf(infiniteValue.xy));
    }
    else
    {
        _71 = false;
    }
    bool _80 = false;
    if (_71)
    {
        _80 = all(isinf(infiniteValue.xyz));
    }
    else
    {
        _80 = false;
    }
    bool _87 = false;
    if (_80)
    {
        _87 = all(isinf(infiniteValue));
    }
    else
    {
        _87 = false;
    }
    bool _94 = false;
    if (_87)
    {
        _94 = !isinf(finiteValue.x);
    }
    else
    {
        _94 = false;
    }
    bool _102 = false;
    if (_94)
    {
        _102 = !any(isinf(finiteValue.xy));
    }
    else
    {
        _102 = false;
    }
    bool _110 = false;
    if (_102)
    {
        _110 = !any(isinf(finiteValue.xyz));
    }
    else
    {
        _110 = false;
    }
    bool _117 = false;
    if (_110)
    {
        _117 = !any(isinf(finiteValue));
    }
    else
    {
        _117 = false;
    }
    float4 _118 = 0.0f.xxxx;
    if (_117)
    {
        _118 = _10_colorGreen;
    }
    else
    {
        _118 = _10_colorRed;
    }
    return _118;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
