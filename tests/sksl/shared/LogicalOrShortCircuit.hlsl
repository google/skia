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
    bool _37 = false;
    if (true)
    {
        _37 = true;
    }
    else
    {
        int _34 = 1 + 1;
        y = _34;
        _37 = _34 == 3;
    }
    if (_37)
    {
        bool _46 = false;
        if (true)
        {
            _46 = y == 1;
        }
        else
        {
            _46 = false;
        }
        return _46;
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
    bool _56 = false;
    if (1 == 2)
    {
        _56 = true;
    }
    else
    {
        int _54 = 1 + 1;
        y = _54;
        _56 = _54 == 2;
    }
    if (_56)
    {
        bool _64 = false;
        if (true)
        {
            _64 = y == 2;
        }
        else
        {
            _64 = false;
        }
        return _64;
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
    bool _73 = false;
    if (1 == 2)
    {
        _73 = true;
    }
    else
    {
        int _71 = 1 + 1;
        y = _71;
        _73 = _71 == 3;
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
            _81 = y == 2;
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
    bool _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = true;
    bool _91 = false;
    if (true)
    {
        _91 = TrueFalse_b();
    }
    else
    {
        _91 = false;
    }
    bool _95 = false;
    if (_91)
    {
        _95 = FalseTrue_b();
    }
    else
    {
        _95 = false;
    }
    bool _99 = false;
    if (_95)
    {
        _99 = FalseFalse_b();
    }
    else
    {
        _99 = false;
    }
    float4 _100 = 0.0f.xxxx;
    if (_99)
    {
        _100 = _10_colorGreen;
    }
    else
    {
        _100 = _10_colorRed;
    }
    return _100;
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
