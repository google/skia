cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _24_colorGreen : packoffset(c0);
    float4 _24_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool return_in_one_case_bi(int _40)
{
    int val = 0;
    switch (_40)
    {
        case 1:
        {
            val = 0 + 1;
            return false;
        }
        default:
        {
            val++;
            break;
        }
    }
    return val == 1;
}

bool return_in_default_bi(int _55)
{
    do
    {
        return true;
    } while(false);
}

bool return_in_every_case_bi(int _61)
{
    switch (_61)
    {
        case 1:
        {
            return false;
        }
        default:
        {
            return true;
        }
    }
}

bool return_in_every_case_no_default_bi(int _67)
{
    int val = 0;
    switch (_67)
    {
        case 1:
        {
            return false;
        }
        case 2:
        {
            return true;
        }
    }
    int _74 = 0 + 1;
    val = _74;
    return _74 == 1;
}

bool case_has_break_before_return_bi(int _76)
{
    int val = 0;
    switch (_76)
    {
        case 1:
        {
            break;
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
    int _84 = 0 + 1;
    val = _84;
    return _84 == 1;
}

bool case_has_break_after_return_bi(int _86)
{
    switch (_86)
    {
        case 1:
        {
            return false;
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

bool no_return_in_default_bi(int _93)
{
    int val = 0;
    switch (_93)
    {
        case 1:
        {
            return false;
        }
        case 2:
        {
            return true;
        }
        default:
        {
            break;
        }
    }
    int _101 = 0 + 1;
    val = _101;
    return _101 == 1;
}

bool empty_default_bi(int _103)
{
    int val = 0;
    switch (_103)
    {
        case 1:
        {
            return false;
        }
        case 2:
        {
            return true;
        }
        default:
        {
            break;
        }
    }
    int _111 = 0 + 1;
    val = _111;
    return _111 == 1;
}

bool return_with_fallthrough_bi(int _113)
{
    switch (_113)
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
            return false;
        }
    }
}

bool fallthrough_ends_in_break_bi(int _120)
{
    int val = 0;
    switch (_120)
    {
        case 1:
        {
            break;
        }
        case 2:
        {
            break;
        }
        default:
        {
            return false;
        }
    }
    int _128 = 0 + 1;
    val = _128;
    return _128 == 1;
}

bool fallthrough_to_default_with_break_bi(int _130)
{
    int val = 0;
    switch (_130)
    {
        case 1:
        {
            break;
        }
        case 2:
        {
            break;
        }
        default:
        {
            break;
        }
    }
    int _138 = 0 + 1;
    val = _138;
    return _138 == 1;
}

bool fallthrough_to_default_with_return_bi(int _140)
{
    switch (_140)
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

bool fallthrough_with_loop_break_bi(int _147)
{
    int val = 0;
    switch (_147)
    {
        case 1:
        {
            for (int i = 0; i < 5; i++)
            {
                val++;
                break;
            }
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

bool fallthrough_with_loop_continue_bi(int _168)
{
    int val = 0;
    switch (_168)
    {
        case 1:
        {
            for (int i = 0; i < 5; i++)
            {
                val++;
            }
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

float4 main(float2 _189)
{
    int _196 = int(_24_colorGreen.y);
    int x = _196;
    int _197 = _196;
    bool _203 = false;
    if (return_in_one_case_bi(_197))
    {
        int _201 = _196;
        _203 = return_in_default_bi(_201);
    }
    else
    {
        _203 = false;
    }
    bool _208 = false;
    if (_203)
    {
        int _206 = _196;
        _208 = return_in_every_case_bi(_206);
    }
    else
    {
        _208 = false;
    }
    bool _213 = false;
    if (_208)
    {
        int _211 = _196;
        _213 = return_in_every_case_no_default_bi(_211);
    }
    else
    {
        _213 = false;
    }
    bool _218 = false;
    if (_213)
    {
        int _216 = _196;
        _218 = case_has_break_before_return_bi(_216);
    }
    else
    {
        _218 = false;
    }
    bool _223 = false;
    if (_218)
    {
        int _221 = _196;
        _223 = case_has_break_after_return_bi(_221);
    }
    else
    {
        _223 = false;
    }
    bool _228 = false;
    if (_223)
    {
        int _226 = _196;
        _228 = no_return_in_default_bi(_226);
    }
    else
    {
        _228 = false;
    }
    bool _233 = false;
    if (_228)
    {
        int _231 = _196;
        _233 = empty_default_bi(_231);
    }
    else
    {
        _233 = false;
    }
    bool _238 = false;
    if (_233)
    {
        int _236 = _196;
        _238 = return_with_fallthrough_bi(_236);
    }
    else
    {
        _238 = false;
    }
    bool _243 = false;
    if (_238)
    {
        int _241 = _196;
        _243 = fallthrough_ends_in_break_bi(_241);
    }
    else
    {
        _243 = false;
    }
    bool _248 = false;
    if (_243)
    {
        int _246 = _196;
        _248 = fallthrough_to_default_with_break_bi(_246);
    }
    else
    {
        _248 = false;
    }
    bool _253 = false;
    if (_248)
    {
        int _251 = _196;
        _253 = fallthrough_to_default_with_return_bi(_251);
    }
    else
    {
        _253 = false;
    }
    bool _258 = false;
    if (_253)
    {
        int _256 = _196;
        _258 = fallthrough_with_loop_break_bi(_256);
    }
    else
    {
        _258 = false;
    }
    bool _263 = false;
    if (_258)
    {
        int _261 = _196;
        _263 = fallthrough_with_loop_continue_bi(_261);
    }
    else
    {
        _263 = false;
    }
    float4 _264 = 0.0f.xxxx;
    if (_263)
    {
        _264 = _24_colorGreen;
    }
    else
    {
        _264 = _24_colorRed;
    }
    return _264;
}

void frag_main()
{
    float2 _34 = 0.0f.xx;
    sk_FragColor = main(_34);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
