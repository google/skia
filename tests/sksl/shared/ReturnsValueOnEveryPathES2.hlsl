cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _15_colorGreen : packoffset(c0);
    float4 _15_colorRed : packoffset(c1);
    float _15_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool return_on_both_sides_b()
{
    if (_15_unknownInput == 1.0f)
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
        if (_15_unknownInput == 1.0f)
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
    if (_15_unknownInput == 1.0f)
    {
        return true;
    }
    else
    {
        if (_15_unknownInput == 2.0f)
        {
            return false;
        }
        else
        {
            if (_15_unknownInput == 3.0f)
            {
                return true;
            }
            else
            {
                if (_15_unknownInput == 4.0f)
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

float4 main(float2 _114)
{
    bool _119 = false;
    if (true)
    {
        _119 = return_on_both_sides_b();
    }
    else
    {
        _119 = false;
    }
    bool _123 = false;
    if (_119)
    {
        _123 = for_inside_body_b();
    }
    else
    {
        _123 = false;
    }
    bool _127 = false;
    if (_123)
    {
        _127 = after_for_body_b();
    }
    else
    {
        _127 = false;
    }
    bool _131 = false;
    if (_127)
    {
        _131 = for_with_double_sided_conditional_return_b();
    }
    else
    {
        _131 = false;
    }
    bool _135 = false;
    if (_131)
    {
        _135 = if_else_chain_b();
    }
    else
    {
        _135 = false;
    }
    float4 _136 = 0.0f.xxxx;
    if (_135)
    {
        _136 = _15_colorGreen;
    }
    else
    {
        _136 = _15_colorRed;
    }
    return _136;
}

void frag_main()
{
    float2 _25 = 0.0f.xx;
    sk_FragColor = main(_25);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
