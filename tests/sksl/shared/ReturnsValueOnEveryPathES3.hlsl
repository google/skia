cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _22_colorGreen : packoffset(c0);
    float4 _22_colorRed : packoffset(c1);
    float _22_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool inside_while_loop_b()
{
    while (_22_unknownInput == 123.0f)
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
    switch (int(_22_unknownInput))
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
    switch (int(_22_unknownInput))
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

bool switch_fallthrough_twice_b()
{
    switch (int(_22_unknownInput))
    {
        case 1:
        {
            return true;
        }
        case 2:
        {
            return true;
        }
        default:
        {
            return true;
        }
    }
}

bool switch_with_break_in_loop_b()
{
    switch (int(_22_unknownInput))
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
    switch (int(_22_unknownInput))
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
    switch (int(_22_unknownInput))
    {
        case 1:
        {
            if (_22_unknownInput == 123.0f)
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
    switch (int(_22_unknownInput))
    {
        case 1:
        {
            if (_22_unknownInput == 123.0f)
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

float4 main(float2 _163)
{
    bool _169 = false;
    if (inside_while_loop_b())
    {
        _169 = inside_infinite_do_loop_b();
    }
    else
    {
        _169 = false;
    }
    bool _173 = false;
    if (_169)
    {
        _173 = inside_infinite_while_loop_b();
    }
    else
    {
        _173 = false;
    }
    bool _177 = false;
    if (_173)
    {
        _177 = after_do_loop_b();
    }
    else
    {
        _177 = false;
    }
    bool _181 = false;
    if (_177)
    {
        _181 = after_while_loop_b();
    }
    else
    {
        _181 = false;
    }
    bool _185 = false;
    if (_181)
    {
        _185 = switch_with_all_returns_b();
    }
    else
    {
        _185 = false;
    }
    bool _189 = false;
    if (_185)
    {
        _189 = switch_fallthrough_b();
    }
    else
    {
        _189 = false;
    }
    bool _193 = false;
    if (_189)
    {
        _193 = switch_fallthrough_twice_b();
    }
    else
    {
        _193 = false;
    }
    bool _197 = false;
    if (_193)
    {
        _197 = switch_with_break_in_loop_b();
    }
    else
    {
        _197 = false;
    }
    bool _201 = false;
    if (_197)
    {
        _201 = switch_with_continue_in_loop_b();
    }
    else
    {
        _201 = false;
    }
    bool _205 = false;
    if (_201)
    {
        _205 = switch_with_if_that_returns_b();
    }
    else
    {
        _205 = false;
    }
    bool _209 = false;
    if (_205)
    {
        _209 = switch_with_one_sided_if_then_fallthrough_b();
    }
    else
    {
        _209 = false;
    }
    float4 _210 = 0.0f.xxxx;
    if (_209)
    {
        _210 = _22_colorGreen;
    }
    else
    {
        _210 = _22_colorRed;
    }
    return _210;
}

void frag_main()
{
    float2 _32 = 0.0f.xx;
    sk_FragColor = main(_32);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
