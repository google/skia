cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _19_colorGreen : packoffset(c0);
    float4 _19_colorRed : packoffset(c1);
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
        if (_19_colorGreen.y > 0.0f)
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
        if (_19_colorGreen.y > 0.0f)
        {
            break;
        }
        else
        {
            continue;
        }
    } while (false);
    return true;
}

bool test_else_b()
{
    do
    {
        if (_19_colorGreen.y == 0.0f)
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

float4 main(float2 _110)
{
    bool _116 = false;
    if (test_return_b())
    {
        _116 = test_break_b();
    }
    else
    {
        _116 = false;
    }
    bool _120 = false;
    if (_116)
    {
        _120 = test_continue_b();
    }
    else
    {
        _120 = false;
    }
    bool _124 = false;
    if (_120)
    {
        _124 = test_if_return_b();
    }
    else
    {
        _124 = false;
    }
    bool _128 = false;
    if (_124)
    {
        _128 = test_if_break_b();
    }
    else
    {
        _128 = false;
    }
    bool _132 = false;
    if (_128)
    {
        _132 = test_else_b();
    }
    else
    {
        _132 = false;
    }
    bool _136 = false;
    if (_132)
    {
        _136 = test_loop_return_b();
    }
    else
    {
        _136 = false;
    }
    bool _140 = false;
    if (_136)
    {
        _140 = test_loop_break_b();
    }
    else
    {
        _140 = false;
    }
    float4 _141 = 0.0f.xxxx;
    if (_140)
    {
        _141 = _19_colorGreen;
    }
    else
    {
        _141 = _19_colorRed;
    }
    return _141;
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
