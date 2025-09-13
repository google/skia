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
    bool _41 = false;
    if (true)
    {
        int _38 = 1 + 1;
        y = _38;
        _41 = _38 == 3;
    }
    else
    {
        _41 = false;
    }
    if (_41)
    {
        return false;
    }
    else
    {
        bool _50 = false;
        if (true)
        {
            _50 = y == 2;
        }
        else
        {
            _50 = false;
        }
        return _50;
    }
}

bool FalseTrue_b()
{
    int x = 1;
    int y = 1;
    bool _59 = false;
    if (1 == 2)
    {
        int _57 = 1 + 1;
        y = _57;
        _59 = _57 == 2;
    }
    else
    {
        _59 = false;
    }
    if (_59)
    {
        return false;
    }
    else
    {
        bool _67 = false;
        if (true)
        {
            _67 = y == 1;
        }
        else
        {
            _67 = false;
        }
        return _67;
    }
}

bool FalseFalse_b()
{
    int x = 1;
    int y = 1;
    bool _76 = false;
    if (1 == 2)
    {
        int _74 = 1 + 1;
        y = _74;
        _76 = _74 == 3;
    }
    else
    {
        _76 = false;
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
            _84 = y == 1;
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
    int _91 = 1 + 1;
    _RESERVED_IDENTIFIER_FIXUP_2_y = _91;
    bool _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = false;
    if (_91 == 2)
    {
        _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = _91 == 2;
    }
    else
    {
        _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = false;
    }
    bool _101 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_TrueTrue)
    {
        _101 = TrueFalse_b();
    }
    else
    {
        _101 = false;
    }
    bool _105 = false;
    if (_101)
    {
        _105 = FalseTrue_b();
    }
    else
    {
        _105 = false;
    }
    bool _109 = false;
    if (_105)
    {
        _109 = FalseFalse_b();
    }
    else
    {
        _109 = false;
    }
    float4 _110 = 0.0f.xxxx;
    if (_109)
    {
        _110 = _14_colorGreen;
    }
    else
    {
        _110 = _14_colorRed;
    }
    return _110;
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
