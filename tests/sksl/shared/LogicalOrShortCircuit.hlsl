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
    bool _39 = false;
    if (true)
    {
        _39 = true;
    }
    else
    {
        int _36 = 1 + 1;
        y = _36;
        _39 = _36 == 3;
    }
    if (_39)
    {
        bool _48 = false;
        if (true)
        {
            _48 = y == 1;
        }
        else
        {
            _48 = false;
        }
        return _48;
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
    bool _58 = false;
    if (1 == 2)
    {
        _58 = true;
    }
    else
    {
        int _56 = 1 + 1;
        y = _56;
        _58 = _56 == 2;
    }
    if (_58)
    {
        bool _66 = false;
        if (true)
        {
            _66 = y == 2;
        }
        else
        {
            _66 = false;
        }
        return _66;
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
    bool _75 = false;
    if (1 == 2)
    {
        _75 = true;
    }
    else
    {
        int _73 = 1 + 1;
        y = _73;
        _75 = _73 == 3;
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
            _83 = y == 2;
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
    bool _RESERVED_IDENTIFIER_FIXUP_0_TrueTrue = true;
    bool _93 = false;
    if (true)
    {
        _93 = TrueFalse_b();
    }
    else
    {
        _93 = false;
    }
    bool _97 = false;
    if (_93)
    {
        _97 = FalseTrue_b();
    }
    else
    {
        _97 = false;
    }
    bool _101 = false;
    if (_97)
    {
        _101 = FalseFalse_b();
    }
    else
    {
        _101 = false;
    }
    float4 _102 = 0.0f.xxxx;
    if (_101)
    {
        _102 = _13_colorGreen;
    }
    else
    {
        _102 = _13_colorRed;
    }
    return _102;
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
