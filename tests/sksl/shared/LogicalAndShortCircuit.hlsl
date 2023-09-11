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

bool TrueFalse_b()
{
    int x = 1;
    int y = 1;
    bool _38 = false;
    if (true)
    {
        int _35 = 1 + 1;
        y = _35;
        _38 = _35 == 3;
    }
    else
    {
        _38 = false;
    }
    if (_38)
    {
        return false;
    }
    else
    {
        bool _47 = false;
        if (true)
        {
            _47 = y == 2;
        }
        else
        {
            _47 = false;
        }
        return _47;
    }
}

bool FalseTrue_b()
{
    int x = 1;
    int y = 1;
    bool _56 = false;
    if (1 == 2)
    {
        int _54 = 1 + 1;
        y = _54;
        _56 = _54 == 2;
    }
    else
    {
        _56 = false;
    }
    if (_56)
    {
        return false;
    }
    else
    {
        bool _64 = false;
        if (true)
        {
            _64 = y == 1;
        }
        else
        {
            _64 = false;
        }
        return _64;
    }
}

bool FalseFalse_b()
{
    int x = 1;
    int y = 1;
    bool _73 = false;
    if (1 == 2)
    {
        int _71 = 1 + 1;
        y = _71;
        _73 = _71 == 3;
    }
    else
    {
        _73 = false;
    }
    if (_73)
    {
        return false;
    }
    else
    {
        bool _81 = false;
        if (true)
        {
            _81 = y == 1;
        }
        else
        {
            _81 = false;
        }
        return _81;
    }
}

float4 main(float2 _83)
{
    int _RESERVED_IDENTIFIER_FIXUP_2_y = 1;
    int _88 = 1 + 1;
    _RESERVED_IDENTIFIER_FIXUP_2_y = _88;
    bool _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = false;
    if (_88 == 2)
    {
        _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = _88 == 2;
    }
    else
    {
        _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = false;
    }
    bool _98 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_TrueTrue)
    {
        _98 = TrueFalse_b();
    }
    else
    {
        _98 = false;
    }
    bool _102 = false;
    if (_98)
    {
        _102 = FalseTrue_b();
    }
    else
    {
        _102 = false;
    }
    bool _106 = false;
    if (_102)
    {
        _106 = FalseFalse_b();
    }
    else
    {
        _106 = false;
    }
    float4 _107 = 0.0f.xxxx;
    if (_106)
    {
        _107 = _10_colorGreen;
    }
    else
    {
        _107 = _10_colorRed;
    }
    return _107;
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
