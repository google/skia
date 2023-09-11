cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _7_testMatrix2x2 : packoffset(c0);
    float4 _7_colorGreen : packoffset(c2);
    float4 _7_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    float4 _43 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y) * (1.0f / _7_colorGreen.x);
    float4 infiniteValue = _43;
    float4 _56 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y) * (1.0f / _7_colorGreen.y);
    float4 finiteValue = _56;
    bool _67 = false;
    if (isinf(_43.x))
    {
        _67 = all(isinf(_43.xy));
    }
    else
    {
        _67 = false;
    }
    bool _75 = false;
    if (_67)
    {
        _75 = all(isinf(_43.xyz));
    }
    else
    {
        _75 = false;
    }
    bool _81 = false;
    if (_75)
    {
        _81 = all(isinf(_43));
    }
    else
    {
        _81 = false;
    }
    bool _87 = false;
    if (_81)
    {
        _87 = !isinf(_56.x);
    }
    else
    {
        _87 = false;
    }
    bool _94 = false;
    if (_87)
    {
        _94 = !any(isinf(_56.xy));
    }
    else
    {
        _94 = false;
    }
    bool _101 = false;
    if (_94)
    {
        _101 = !any(isinf(_56.xyz));
    }
    else
    {
        _101 = false;
    }
    bool _107 = false;
    if (_101)
    {
        _107 = !any(isinf(_56));
    }
    else
    {
        _107 = false;
    }
    float4 _108 = 0.0f.xxxx;
    if (_107)
    {
        _108 = _7_colorGreen;
    }
    else
    {
        _108 = _7_colorRed;
    }
    return _108;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
