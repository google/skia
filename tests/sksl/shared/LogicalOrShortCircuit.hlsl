cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _14_colorGreen : packoffset(c0);
    float4 _14_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool TrueFalse_b()
{
    int x = 1;
    int y = 1;
    bool _40 = false;
    if (true)
    {
        _40 = true;
    }
    else
    {
        int _37 = 1 + 1;
        y = _37;
        _40 = _37 == 3;
    }
    if (_40)
    {
        bool _49 = false;
        if (true)
        {
            _49 = y == 1;
        }
        else
        {
            _49 = false;
        }
        return _49;
    }
    else
    {
        return false;
    }
}

bool FalseTrue_b()
{
    int x = 1;
    int y = 1;
    bool _59 = false;
    if (1 == 2)
    {
        _59 = true;
    }
    else
    {
        int _57 = 1 + 1;
        y = _57;
        _59 = _57 == 2;
    }
    if (_59)
    {
        bool _67 = false;
        if (true)
        {
            _67 = y == 2;
        }
        else
        {
            _67 = false;
        }
        return _67;
    }
    else
    {
        return false;
    }
}

bool FalseFalse_b()
{
    int x = 1;
    int y = 1;
    bool _76 = false;
    if (1 == 2)
    {
        _76 = true;
    }
    else
    {
        int _74 = 1 + 1;
        y = _74;
        _76 = _74 == 3;
    }
    if (_76)
    {
        return false;
    }
    else
    {
        bool _84 = false;
        if (true)
        {
            _84 = y == 2;
        }
        else
        {
            _84 = false;
        }
        return _84;
    }
}

float4 main(float2 _86)
{
    int _RESERVED_IDENTIFIER_FIXUP_2_y = 1;
    bool _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = true;
    bool _94 = false;
    if (true)
    {
        _94 = TrueFalse_b();
    }
    else
    {
        _94 = false;
    }
    bool _98 = false;
    if (_94)
    {
        _98 = FalseTrue_b();
    }
    else
    {
        _98 = false;
    }
    bool _102 = false;
    if (_98)
    {
        _102 = FalseFalse_b();
    }
    else
    {
        _102 = false;
    }
    float4 _103 = 0.0f.xxxx;
    if (_102)
    {
        _103 = _14_colorGreen;
    }
    else
    {
        _103 = _14_colorRed;
    }
    return _103;
}

void frag_main()
{
    float2 _24 = 0.0f.xx;
    sk_FragColor = main(_24);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
