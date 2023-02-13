cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _13_colorGreen : packoffset(c0);
    float4 _13_colorRed : packoffset(c1);
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
        int _37 = 1 + 1;
        y = _37;
        _40 = _37 == 3;
    }
    else
    {
        _40 = false;
    }
    if (_40)
    {
        return false;
    }
    else
    {
        bool _49 = false;
        if (true)
        {
            _49 = y == 2;
        }
        else
        {
            _49 = false;
        }
        return _49;
    }
}

bool FalseTrue_b()
{
    int x = 1;
    int y = 1;
    bool _58 = false;
    if (1 == 2)
    {
        int _56 = 1 + 1;
        y = _56;
        _58 = _56 == 2;
    }
    else
    {
        _58 = false;
    }
    if (_58)
    {
        return false;
    }
    else
    {
        bool _66 = false;
        if (true)
        {
            _66 = y == 1;
        }
        else
        {
            _66 = false;
        }
        return _66;
    }
}

bool FalseFalse_b()
{
    int x = 1;
    int y = 1;
    bool _75 = false;
    if (1 == 2)
    {
        int _73 = 1 + 1;
        y = _73;
        _75 = _73 == 3;
    }
    else
    {
        _75 = false;
    }
    if (_75)
    {
        return false;
    }
    else
    {
        bool _83 = false;
        if (true)
        {
            _83 = y == 1;
        }
        else
        {
            _83 = false;
        }
        return _83;
    }
}

float4 main(float2 _85)
{
    int _RESERVED_IDENTIFIER_FIXUP_2_y = 1;
    int _90 = 1 + 1;
    _RESERVED_IDENTIFIER_FIXUP_2_y = _90;
    bool _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = false;
    if (_90 == 2)
    {
        _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = _90 == 2;
    }
    else
    {
        _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = false;
    }
    bool _100 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_0_TrueTrue)
    {
        _100 = TrueFalse_b();
    }
    else
    {
        _100 = false;
    }
    bool _104 = false;
    if (_100)
    {
        _104 = FalseTrue_b();
    }
    else
    {
        _104 = false;
    }
    bool _108 = false;
    if (_104)
    {
        _108 = FalseFalse_b();
    }
    else
    {
        _108 = false;
    }
    float4 _109 = 0.0f.xxxx;
    if (_108)
    {
        _109 = _13_colorGreen;
    }
    else
    {
        _109 = _13_colorRed;
    }
    return _109;
}

void frag_main()
{
    float2 _23 = 0.0f.xx;
    sk_FragColor = main(_23);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
