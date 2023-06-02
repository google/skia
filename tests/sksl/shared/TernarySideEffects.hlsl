cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float x = 1.0f;
    float y = 1.0f;
    float _31 = 0.0f;
    if (true)
    {
        float _35 = 1.0f + 1.0f;
        x = _35;
        _31 = _35;
    }
    else
    {
        float _36 = 1.0f + 1.0f;
        y = _36;
        _31 = _36;
    }
    float _41 = 0.0f;
    if (x == y)
    {
        float _45 = x;
        float _47 = _45 + 3.0f;
        x = _47;
        _41 = _47;
    }
    else
    {
        float _48 = y;
        float _49 = _48 + 3.0f;
        y = _49;
        _41 = _49;
    }
    float _54 = 0.0f;
    if (x < y)
    {
        float _58 = x;
        float _60 = _58 + 5.0f;
        x = _60;
        _54 = _60;
    }
    else
    {
        float _61 = y;
        float _62 = _61 + 5.0f;
        y = _62;
        _54 = _62;
    }
    float _67 = 0.0f;
    if (y >= x)
    {
        float _71 = x;
        float _73 = _71 + 9.0f;
        x = _73;
        _67 = _73;
    }
    else
    {
        float _74 = y;
        float _75 = _74 + 9.0f;
        y = _75;
        _67 = _75;
    }
    float _80 = 0.0f;
    if (x != y)
    {
        float _84 = x;
        float _85 = _84 + 1.0f;
        x = _85;
        _80 = _85;
    }
    else
    {
        _80 = y;
    }
    float _91 = 0.0f;
    if (x == y)
    {
        float _95 = x;
        float _97 = _95 + 2.0f;
        x = _97;
        _91 = _97;
    }
    else
    {
        _91 = y;
    }
    float _103 = 0.0f;
    if (x != y)
    {
        _103 = x;
    }
    else
    {
        float _108 = y;
        float _109 = _108 + 3.0f;
        y = _109;
        _103 = _109;
    }
    float _114 = 0.0f;
    if (x == y)
    {
        _114 = x;
    }
    else
    {
        float _119 = y;
        float _121 = _119 + 4.0f;
        y = _121;
        _114 = _121;
    }
    bool b = true;
    b = false;
    bool _127 = false;
    if (false)
    {
        _127 = false;
    }
    else
    {
        _127 = false;
    }
    bool c = _127;
    float4 _132 = 0.0f.xxxx;
    if (_127)
    {
        _132 = _10_colorRed;
    }
    else
    {
        bool _150 = false;
        if (x == 8.0f)
        {
            _150 = y == 17.0f;
        }
        else
        {
            _150 = false;
        }
        float4 _151 = 0.0f.xxxx;
        if (_150)
        {
            _151 = _10_colorGreen;
        }
        else
        {
            _151 = _10_colorRed;
        }
        _132 = _151;
    }
    return _132;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
