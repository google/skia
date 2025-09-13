cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _16_colorGreen : packoffset(c0);
    float4 _16_colorRed : packoffset(c1);
    float _16_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool return_on_both_sides_b()
{
    if (_16_unknownInput == 1.0f)
    {
        return true;
    }
    else
    {
        return true;
    }
}

bool for_inside_body_b()
{
    for (int x = 0; x <= 10; x++)
    {
        return true;
    }
}

bool after_for_body_b()
{
    for (int x = 0; x <= 10; x++)
    {
    }
    return true;
}

bool for_with_double_sided_conditional_return_b()
{
    for (int x = 0; x <= 10; x++)
    {
        if (_16_unknownInput == 1.0f)
        {
            return true;
        }
        else
        {
            return true;
        }
    }
}

bool if_else_chain_b()
{
    if (_16_unknownInput == 1.0f)
    {
        return true;
    }
    else
    {
        if (_16_unknownInput == 2.0f)
        {
            return false;
        }
        else
        {
            if (_16_unknownInput == 3.0f)
            {
                return true;
            }
            else
            {
                if (_16_unknownInput == 4.0f)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
    }
}

float4 main(float2 _115)
{
    bool _120 = false;
    if (true)
    {
        _120 = return_on_both_sides_b();
    }
    else
    {
        _120 = false;
    }
    bool _124 = false;
    if (_120)
    {
        _124 = for_inside_body_b();
    }
    else
    {
        _124 = false;
    }
    bool _128 = false;
    if (_124)
    {
        _128 = after_for_body_b();
    }
    else
    {
        _128 = false;
    }
    bool _132 = false;
    if (_128)
    {
        _132 = for_with_double_sided_conditional_return_b();
    }
    else
    {
        _132 = false;
    }
    bool _136 = false;
    if (_132)
    {
        _136 = if_else_chain_b();
    }
    else
    {
        _136 = false;
    }
    float4 _137 = 0.0f.xxxx;
    if (_136)
    {
        _137 = _16_colorGreen;
    }
    else
    {
        _137 = _16_colorRed;
    }
    return _137;
}

void frag_main()
{
    float2 _26 = 0.0f.xx;
    sk_FragColor = main(_26);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
