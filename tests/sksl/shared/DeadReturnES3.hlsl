cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _15_colorGreen : packoffset(c0);
    float4 _15_colorRed : packoffset(c1);
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
        if (_15_colorGreen.y > 0.0f)
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
        if (_15_colorGreen.y > 0.0f)
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
        if (_15_colorGreen.y == 0.0f)
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

float4 main(float2 _107)
{
    bool _113 = false;
    if (test_return_b())
    {
        _113 = test_break_b();
    }
    else
    {
        _113 = false;
    }
    bool _117 = false;
    if (_113)
    {
        _117 = test_continue_b();
    }
    else
    {
        _117 = false;
    }
    bool _121 = false;
    if (_117)
    {
        _121 = test_if_return_b();
    }
    else
    {
        _121 = false;
    }
    bool _125 = false;
    if (_121)
    {
        _125 = test_if_break_b();
    }
    else
    {
        _125 = false;
    }
    bool _129 = false;
    if (_125)
    {
        _129 = test_else_b();
    }
    else
    {
        _129 = false;
    }
    bool _133 = false;
    if (_129)
    {
        _133 = test_loop_return_b();
    }
    else
    {
        _133 = false;
    }
    bool _137 = false;
    if (_133)
    {
        _137 = test_loop_break_b();
    }
    else
    {
        _137 = false;
    }
    float4 _138 = 0.0f.xxxx;
    if (_137)
    {
        _138 = _15_colorGreen;
    }
    else
    {
        _138 = _15_colorRed;
    }
    return _138;
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
