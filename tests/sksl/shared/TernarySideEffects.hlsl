cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float x = 1.0f;
    float y = 1.0f;
    float _33 = 0.0f;
    if (true)
    {
        float _37 = 1.0f + 1.0f;
        x = _37;
        _33 = _37;
    }
    else
    {
        float _38 = 1.0f + 1.0f;
        y = _38;
        _33 = _38;
    }
    float _43 = 0.0f;
    if (x == y)
    {
        float _47 = x;
        float _49 = _47 + 3.0f;
        x = _49;
        _43 = _49;
    }
    else
    {
        float _50 = y;
        float _51 = _50 + 3.0f;
        y = _51;
        _43 = _51;
    }
    float _56 = 0.0f;
    if (x < y)
    {
        float _60 = x;
        float _62 = _60 + 5.0f;
        x = _62;
        _56 = _62;
    }
    else
    {
        float _63 = y;
        float _64 = _63 + 5.0f;
        y = _64;
        _56 = _64;
    }
    float _69 = 0.0f;
    if (y >= x)
    {
        float _73 = x;
        float _75 = _73 + 9.0f;
        x = _75;
        _69 = _75;
    }
    else
    {
        float _76 = y;
        float _77 = _76 + 9.0f;
        y = _77;
        _69 = _77;
    }
    float _82 = 0.0f;
    if (x != y)
    {
        float _86 = x;
        float _87 = _86 + 1.0f;
        x = _87;
        _82 = _87;
    }
    else
    {
        _82 = y;
    }
    float _93 = 0.0f;
    if (x == y)
    {
        float _97 = x;
        float _99 = _97 + 2.0f;
        x = _99;
        _93 = _99;
    }
    else
    {
        _93 = y;
    }
    float _105 = 0.0f;
    if (x != y)
    {
        _105 = x;
    }
    else
    {
        float _110 = y;
        float _111 = _110 + 3.0f;
        y = _111;
        _105 = _111;
    }
    float _116 = 0.0f;
    if (x == y)
    {
        _116 = x;
    }
    else
    {
        float _121 = y;
        float _123 = _121 + 4.0f;
        y = _123;
        _116 = _123;
    }
    bool b = true;
    b = false;
    bool _129 = false;
    if (false)
    {
        _129 = false;
    }
    else
    {
        _129 = false;
    }
    bool c = _129;
    float4 _134 = 0.0f.xxxx;
    if (_129)
    {
        _134 = _11_colorRed;
    }
    else
    {
        bool _151 = false;
        if (x == 8.0f)
        {
            _151 = y == 17.0f;
        }
        else
        {
            _151 = false;
        }
        float4 _152 = 0.0f.xxxx;
        if (_151)
        {
            _152 = _11_colorGreen;
        }
        else
        {
            _152 = _11_colorRed;
        }
        _134 = _152;
    }
    return _134;
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
