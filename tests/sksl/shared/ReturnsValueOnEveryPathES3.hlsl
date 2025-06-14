cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _23_colorGreen : packoffset(c0);
    float4 _23_colorRed : packoffset(c1);
    float _23_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool inside_while_loop_b()
{
    while (_23_unknownInput == 123.0f)
    {
        return false;
    }
    return true;
}

bool inside_infinite_do_loop_b()
{
    do
    {
        return true;
    } while (true);
}

bool inside_infinite_while_loop_b()
{
    while (true)
    {
        return true;
    }
}

bool after_do_loop_b()
{
    do
    {
        break;
    } while (true);
    return true;
}

bool after_while_loop_b()
{
    while (!true)
    {
        break;
    }
    return true;
}

bool switch_with_all_returns_b()
{
    switch (int(_23_unknownInput))
    {
        case 1:
        {
            return true;
        }
        case 2:
        {
            return false;
        }
        default:
        {
            return false;
        }
    }
}

bool switch_fallthrough_b()
{
    switch (int(_23_unknownInput))
    {
        case 1:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

bool switch_fallthrough_twice_b()
{
    switch (int(_23_unknownInput))
    {
        default:
        {
            return true;
        }
    }
}

bool switch_with_break_in_loop_b()
{
    switch (int(_23_unknownInput))
    {
        case 1:
        {
            for (int x = 0; !(x <= 10); x++)
            {
                break;
            }
            return true;
        }
        default:
        {
            return true;
        }
    }
}

bool switch_with_continue_in_loop_b()
{
    switch (int(_23_unknownInput))
    {
        case 1:
        {
            for (int x = 0; x <= 10; x++)
            {
            }
            return true;
        }
        default:
        {
            return true;
        }
    }
}

bool switch_with_if_that_returns_b()
{
    switch (int(_23_unknownInput))
    {
        case 1:
        {
            if (_23_unknownInput == 123.0f)
            {
                return false;
            }
            else
            {
                return true;
            }
            return true;
        }
        default:
        {
            return true;
        }
    }
}

bool switch_with_one_sided_if_then_fallthrough_b()
{
    switch (int(_23_unknownInput))
    {
        case 1:
        {
            if (_23_unknownInput == 123.0f)
            {
                return false;
            }
            return true;
        }
        default:
        {
            return true;
        }
    }
}

float4 main(float2 _164)
{
    bool _170 = false;
    if (inside_while_loop_b())
    {
        _170 = inside_infinite_do_loop_b();
    }
    else
    {
        _170 = false;
    }
    bool _174 = false;
    if (_170)
    {
        _174 = inside_infinite_while_loop_b();
    }
    else
    {
        _174 = false;
    }
    bool _178 = false;
    if (_174)
    {
        _178 = after_do_loop_b();
    }
    else
    {
        _178 = false;
    }
    bool _182 = false;
    if (_178)
    {
        _182 = after_while_loop_b();
    }
    else
    {
        _182 = false;
    }
    bool _186 = false;
    if (_182)
    {
        _186 = switch_with_all_returns_b();
    }
    else
    {
        _186 = false;
    }
    bool _190 = false;
    if (_186)
    {
        _190 = switch_fallthrough_b();
    }
    else
    {
        _190 = false;
    }
    bool _194 = false;
    if (_190)
    {
        _194 = switch_fallthrough_twice_b();
    }
    else
    {
        _194 = false;
    }
    bool _198 = false;
    if (_194)
    {
        _198 = switch_with_break_in_loop_b();
    }
    else
    {
        _198 = false;
    }
    bool _202 = false;
    if (_198)
    {
        _202 = switch_with_continue_in_loop_b();
    }
    else
    {
        _202 = false;
    }
    bool _206 = false;
    if (_202)
    {
        _206 = switch_with_if_that_returns_b();
    }
    else
    {
        _206 = false;
    }
    bool _210 = false;
    if (_206)
    {
        _210 = switch_with_one_sided_if_then_fallthrough_b();
    }
    else
    {
        _210 = false;
    }
    float4 _211 = 0.0f.xxxx;
    if (_210)
    {
        _211 = _23_colorGreen;
    }
    else
    {
        _211 = _23_colorRed;
    }
    return _211;
}

void frag_main()
{
    float2 _33 = 0.0f.xx;
    sk_FragColor = main(_33);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
