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
    float4 _46 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * (1.0f / _10_colorGreen.x);
    float4 infiniteValue = _46;
    float4 _59 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) * (1.0f / _10_colorGreen.y);
    float4 finiteValue = _59;
    bool _69 = false;
    if (isinf(_46.x))
    {
        _69 = all(isinf(_46.xy));
    }
    else
    {
        _69 = false;
    }
    bool _77 = false;
    if (_69)
    {
        _77 = all(isinf(_46.xyz));
    }
    else
    {
        _77 = false;
    }
    bool _83 = false;
    if (_77)
    {
        _83 = all(isinf(_46));
    }
    else
    {
        _83 = false;
    }
    bool _89 = false;
    if (_83)
    {
        _89 = !isinf(_59.x);
    }
    else
    {
        _89 = false;
    }
    bool _96 = false;
    if (_89)
    {
        _96 = !any(isinf(_59.xy));
    }
    else
    {
        _96 = false;
    }
    bool _103 = false;
    if (_96)
    {
        _103 = !any(isinf(_59.xyz));
    }
    else
    {
        _103 = false;
    }
    bool _109 = false;
    if (_103)
    {
        _109 = !any(isinf(_59));
    }
    else
    {
        _109 = false;
    }
    float4 _110 = 0.0f.xxxx;
    if (_109)
    {
        _110 = _10_colorGreen;
    }
    else
    {
        _110 = _10_colorRed;
    }
    return _110;
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
