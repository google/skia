cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testMatrix2x2 : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 _38 = _11_testMatrix2x2 + float4(2.0f, -2.0f, 1.0f, 8.0f);
    float4 inputVal = _38;
    float4 expected = float4(3.0f, 3.0f, 5.0f, 13.0f);
    bool _59 = false;
    if (abs(length(_38.x) - 3.0f) < 0.0500000007450580596923828125f)
    {
        _59 = abs(length(_38.xy) - 3.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _59 = false;
    }
    bool _68 = false;
    if (_59)
    {
        _68 = abs(length(_38.xyz) - 5.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _68 = false;
    }
    bool _75 = false;
    if (_68)
    {
        _75 = abs(length(_38) - 13.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _75 = false;
    }
    bool _81 = false;
    if (_75)
    {
        _81 = abs(3.0f - 3.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _81 = false;
    }
    bool _87 = false;
    if (_81)
    {
        _87 = abs(3.0f - 3.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _87 = false;
    }
    bool _93 = false;
    if (_87)
    {
        _93 = abs(5.0f - 5.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _93 = false;
    }
    bool _99 = false;
    if (_93)
    {
        _99 = abs(13.0f - 13.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _99 = false;
    }
    float4 _100 = 0.0f.xxxx;
    if (_99)
    {
        _100 = _11_colorGreen;
    }
    else
    {
        _100 = _11_colorRed;
    }
    return _100;
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
