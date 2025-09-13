cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _25_colorGreen : packoffset(c0);
    float4 _25_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool return_in_one_case_bi(int _41)
{
    int val = 0;
    switch (_41)
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

bool return_in_default_bi(int _56)
{
    do
    {
        return true;
    } while(false);
}

bool return_in_every_case_bi(int _62)
{
    switch (_62)
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

bool return_in_every_case_no_default_bi(int _68)
{
    int val = 0;
    switch (_68)
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
    int _75 = 0 + 1;
    val = _75;
    return _75 == 1;
}

bool case_has_break_before_return_bi(int _77)
{
    int val = 0;
    switch (_77)
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
    int _85 = 0 + 1;
    val = _85;
    return _85 == 1;
}

bool case_has_break_after_return_bi(int _87)
{
    switch (_87)
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

bool no_return_in_default_bi(int _94)
{
    int val = 0;
    switch (_94)
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
    int _102 = 0 + 1;
    val = _102;
    return _102 == 1;
}

bool empty_default_bi(int _104)
{
    int val = 0;
    switch (_104)
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
    int _112 = 0 + 1;
    val = _112;
    return _112 == 1;
}

bool return_with_fallthrough_bi(int _114)
{
    switch (_114)
    {
        case 1:
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

bool fallthrough_ends_in_break_bi(int _121)
{
    int val = 0;
    switch (_121)
    {
        case 1:
        case 2:
        {
            break;
        }
        default:
        {
            return false;
        }
    }
    int _129 = 0 + 1;
    val = _129;
    return _129 == 1;
}

bool fallthrough_to_default_with_break_bi(int _131)
{
    int val = 0;
    switch (_131)
    {
        default:
        {
            break;
        }
    }
    int _139 = 0 + 1;
    val = _139;
    return _139 == 1;
}

bool fallthrough_to_default_with_return_bi(int _141)
{
    switch (_141)
    {
        default:
        {
            return true;
        }
    }
}

bool fallthrough_with_loop_break_bi(int _148)
{
    int val = 0;
    switch (_148)
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
        default:
        {
            return true;
        }
    }
}

bool fallthrough_with_loop_continue_bi(int _169)
{
    int val = 0;
    switch (_169)
    {
        case 1:
        {
            for (int i = 0; i < 5; i++)
            {
                val++;
            }
            return true;
        }
        default:
        {
            return true;
        }
    }
}

float4 main(float2 _190)
{
    int _197 = int(_25_colorGreen.y);
    int x = _197;
    int _198 = _197;
    bool _204 = false;
    if (return_in_one_case_bi(_198))
    {
        int _202 = _197;
        _204 = return_in_default_bi(_202);
    }
    else
    {
        _204 = false;
    }
    bool _209 = false;
    if (_204)
    {
        int _207 = _197;
        _209 = return_in_every_case_bi(_207);
    }
    else
    {
        _209 = false;
    }
    bool _214 = false;
    if (_209)
    {
        int _212 = _197;
        _214 = return_in_every_case_no_default_bi(_212);
    }
    else
    {
        _214 = false;
    }
    bool _219 = false;
    if (_214)
    {
        int _217 = _197;
        _219 = case_has_break_before_return_bi(_217);
    }
    else
    {
        _219 = false;
    }
    bool _224 = false;
    if (_219)
    {
        int _222 = _197;
        _224 = case_has_break_after_return_bi(_222);
    }
    else
    {
        _224 = false;
    }
    bool _229 = false;
    if (_224)
    {
        int _227 = _197;
        _229 = no_return_in_default_bi(_227);
    }
    else
    {
        _229 = false;
    }
    bool _234 = false;
    if (_229)
    {
        int _232 = _197;
        _234 = empty_default_bi(_232);
    }
    else
    {
        _234 = false;
    }
    bool _239 = false;
    if (_234)
    {
        int _237 = _197;
        _239 = return_with_fallthrough_bi(_237);
    }
    else
    {
        _239 = false;
    }
    bool _244 = false;
    if (_239)
    {
        int _242 = _197;
        _244 = fallthrough_ends_in_break_bi(_242);
    }
    else
    {
        _244 = false;
    }
    bool _249 = false;
    if (_244)
    {
        int _247 = _197;
        _249 = fallthrough_to_default_with_break_bi(_247);
    }
    else
    {
        _249 = false;
    }
    bool _254 = false;
    if (_249)
    {
        int _252 = _197;
        _254 = fallthrough_to_default_with_return_bi(_252);
    }
    else
    {
        _254 = false;
    }
    bool _259 = false;
    if (_254)
    {
        int _257 = _197;
        _259 = fallthrough_with_loop_break_bi(_257);
    }
    else
    {
        _259 = false;
    }
    bool _264 = false;
    if (_259)
    {
        int _262 = _197;
        _264 = fallthrough_with_loop_continue_bi(_262);
    }
    else
    {
        _264 = false;
    }
    float4 _265 = 0.0f.xxxx;
    if (_264)
    {
        _265 = _25_colorGreen;
    }
    else
    {
        _265 = _25_colorRed;
    }
    return _265;
}

void frag_main()
{
    float2 _35 = 0.0f.xx;
    sk_FragColor = main(_35);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
