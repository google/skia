cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
    float _12_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool return_on_both_sides_b()
{
    if (_12_unknownInput == 1.0f)
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
        if (_12_unknownInput == 1.0f)
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
    if (_12_unknownInput == 1.0f)
    {
        return true;
    }
    else
    {
        if (_12_unknownInput == 2.0f)
        {
            return false;
        }
        else
        {
            if (_12_unknownInput == 3.0f)
            {
                return true;
            }
            else
            {
                if (_12_unknownInput == 4.0f)
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

float4 main(float2 _112)
{
    bool _117 = false;
    if (true)
    {
        _117 = return_on_both_sides_b();
    }
    else
    {
        _117 = false;
    }
    bool _121 = false;
    if (_117)
    {
        _121 = for_inside_body_b();
    }
    else
    {
        _121 = false;
    }
    bool _125 = false;
    if (_121)
    {
        _125 = after_for_body_b();
    }
    else
    {
        _125 = false;
    }
    bool _129 = false;
    if (_125)
    {
        _129 = for_with_double_sided_conditional_return_b();
    }
    else
    {
        _129 = false;
    }
    bool _133 = false;
    if (_129)
    {
        _133 = if_else_chain_b();
    }
    else
    {
        _133 = false;
    }
    float4 _134 = 0.0f.xxxx;
    if (_133)
    {
        _134 = _12_colorGreen;
    }
    else
    {
        _134 = _12_colorRed;
    }
    return _134;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
