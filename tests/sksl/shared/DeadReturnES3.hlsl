cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _18_colorGreen : packoffset(c0);
    float4 _18_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_return_b()
{
    do
    {
        return true;
    } while (false);
}

bool test_break_b()
{
    do
    {
        break;
    } while (false);
    return true;
}

bool test_continue_b()
{
    do
    {
    } while (false);
    return true;
}

bool test_if_return_b()
{
    do
    {
        if (_18_colorGreen.y > 0.0f)
        {
            return true;
        }
        else
        {
            break;
        }
    } while (false);
    return false;
}

bool test_if_break_b()
{
    do
    {
        if (_18_colorGreen.y > 0.0f)
        {
            break;
        }
        else
        {
        }
    } while (false);
    return true;
}

bool test_else_b()
{
    do
    {
        if (_18_colorGreen.y == 0.0f)
        {
            return false;
        }
        else
        {
            return true;
        }
    } while (false);
}

bool test_loop_return_b()
{
    return true;
}

bool test_loop_break_b()
{
    for (int x = 0; !(x <= 1); x++)
    {
        break;
    }
    return true;
}

float4 main(float2 _109)
{
    bool _115 = false;
    if (test_return_b())
    {
        _115 = test_break_b();
    }
    else
    {
        _115 = false;
    }
    bool _119 = false;
    if (_115)
    {
        _119 = test_continue_b();
    }
    else
    {
        _119 = false;
    }
    bool _123 = false;
    if (_119)
    {
        _123 = test_if_return_b();
    }
    else
    {
        _123 = false;
    }
    bool _127 = false;
    if (_123)
    {
        _127 = test_if_break_b();
    }
    else
    {
        _127 = false;
    }
    bool _131 = false;
    if (_127)
    {
        _131 = test_else_b();
    }
    else
    {
        _131 = false;
    }
    bool _135 = false;
    if (_131)
    {
        _135 = test_loop_return_b();
    }
    else
    {
        _135 = false;
    }
    bool _139 = false;
    if (_135)
    {
        _139 = test_loop_break_b();
    }
    else
    {
        _139 = false;
    }
    float4 _140 = 0.0f.xxxx;
    if (_139)
    {
        _140 = _18_colorGreen;
    }
    else
    {
        _140 = _18_colorRed;
    }
    return _140;
}

void frag_main()
{
    float2 _28 = 0.0f.xx;
    sk_FragColor = main(_28);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
