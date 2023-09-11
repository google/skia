cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _19_colorGreen : packoffset(c0);
    float4 _19_colorRed : packoffset(c1);
    float _19_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool inside_while_loop_b()
{
    while (_19_unknownInput == 123.0f)
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
    switch (int(_19_unknownInput))
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
    switch (int(_19_unknownInput))
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
    switch (int(_19_unknownInput))
    {
        default:
        {
            return true;
        }
    }
}

bool switch_with_break_in_loop_b()
{
    switch (int(_19_unknownInput))
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
    switch (int(_19_unknownInput))
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
    switch (int(_19_unknownInput))
    {
        case 1:
        {
            if (_19_unknownInput == 123.0f)
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
    switch (int(_19_unknownInput))
    {
        case 1:
        {
            if (_19_unknownInput == 123.0f)
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

float4 main(float2 _161)
{
    bool _167 = false;
    if (inside_while_loop_b())
    {
        _167 = inside_infinite_do_loop_b();
    }
    else
    {
        _167 = false;
    }
    bool _171 = false;
    if (_167)
    {
        _171 = inside_infinite_while_loop_b();
    }
    else
    {
        _171 = false;
    }
    bool _175 = false;
    if (_171)
    {
        _175 = after_do_loop_b();
    }
    else
    {
        _175 = false;
    }
    bool _179 = false;
    if (_175)
    {
        _179 = after_while_loop_b();
    }
    else
    {
        _179 = false;
    }
    bool _183 = false;
    if (_179)
    {
        _183 = switch_with_all_returns_b();
    }
    else
    {
        _183 = false;
    }
    bool _187 = false;
    if (_183)
    {
        _187 = switch_fallthrough_b();
    }
    else
    {
        _187 = false;
    }
    bool _191 = false;
    if (_187)
    {
        _191 = switch_fallthrough_twice_b();
    }
    else
    {
        _191 = false;
    }
    bool _195 = false;
    if (_191)
    {
        _195 = switch_with_break_in_loop_b();
    }
    else
    {
        _195 = false;
    }
    bool _199 = false;
    if (_195)
    {
        _199 = switch_with_continue_in_loop_b();
    }
    else
    {
        _199 = false;
    }
    bool _203 = false;
    if (_199)
    {
        _203 = switch_with_if_that_returns_b();
    }
    else
    {
        _203 = false;
    }
    bool _207 = false;
    if (_203)
    {
        _207 = switch_with_one_sided_if_then_fallthrough_b();
    }
    else
    {
        _207 = false;
    }
    float4 _208 = 0.0f.xxxx;
    if (_207)
    {
        _208 = _19_colorGreen;
    }
    else
    {
        _208 = _19_colorRed;
    }
    return _208;
}

void frag_main()
{
    float2 _29 = 0.0f.xx;
    sk_FragColor = main(_29);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
