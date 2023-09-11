cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _21_colorGreen : packoffset(c0);
    float4 _21_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool return_in_one_case_bi(int _38)
{
    int val = 0;
    switch (_38)
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

bool return_in_default_bi(int _53)
{
    do
    {
        return true;
    } while(false);
}

bool return_in_every_case_bi(int _59)
{
    switch (_59)
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

bool return_in_every_case_no_default_bi(int _65)
{
    int val = 0;
    switch (_65)
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
    int _72 = 0 + 1;
    val = _72;
    return _72 == 1;
}

bool case_has_break_before_return_bi(int _74)
{
    int val = 0;
    switch (_74)
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
    int _82 = 0 + 1;
    val = _82;
    return _82 == 1;
}

bool case_has_break_after_return_bi(int _84)
{
    switch (_84)
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

bool no_return_in_default_bi(int _91)
{
    int val = 0;
    switch (_91)
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
    int _99 = 0 + 1;
    val = _99;
    return _99 == 1;
}

bool empty_default_bi(int _101)
{
    int val = 0;
    switch (_101)
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
    int _109 = 0 + 1;
    val = _109;
    return _109 == 1;
}

bool return_with_fallthrough_bi(int _111)
{
    switch (_111)
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

bool fallthrough_ends_in_break_bi(int _118)
{
    int val = 0;
    switch (_118)
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
    int _126 = 0 + 1;
    val = _126;
    return _126 == 1;
}

bool fallthrough_to_default_with_break_bi(int _128)
{
    int val = 0;
    switch (_128)
    {
        default:
        {
            break;
        }
    }
    int _136 = 0 + 1;
    val = _136;
    return _136 == 1;
}

bool fallthrough_to_default_with_return_bi(int _138)
{
    switch (_138)
    {
        default:
        {
            return true;
        }
    }
}

bool fallthrough_with_loop_break_bi(int _145)
{
    int val = 0;
    switch (_145)
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

bool fallthrough_with_loop_continue_bi(int _166)
{
    int val = 0;
    switch (_166)
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

float4 main(float2 _187)
{
    int _194 = int(_21_colorGreen.y);
    int x = _194;
    int _195 = _194;
    bool _201 = false;
    if (return_in_one_case_bi(_195))
    {
        int _199 = _194;
        _201 = return_in_default_bi(_199);
    }
    else
    {
        _201 = false;
    }
    bool _206 = false;
    if (_201)
    {
        int _204 = _194;
        _206 = return_in_every_case_bi(_204);
    }
    else
    {
        _206 = false;
    }
    bool _211 = false;
    if (_206)
    {
        int _209 = _194;
        _211 = return_in_every_case_no_default_bi(_209);
    }
    else
    {
        _211 = false;
    }
    bool _216 = false;
    if (_211)
    {
        int _214 = _194;
        _216 = case_has_break_before_return_bi(_214);
    }
    else
    {
        _216 = false;
    }
    bool _221 = false;
    if (_216)
    {
        int _219 = _194;
        _221 = case_has_break_after_return_bi(_219);
    }
    else
    {
        _221 = false;
    }
    bool _226 = false;
    if (_221)
    {
        int _224 = _194;
        _226 = no_return_in_default_bi(_224);
    }
    else
    {
        _226 = false;
    }
    bool _231 = false;
    if (_226)
    {
        int _229 = _194;
        _231 = empty_default_bi(_229);
    }
    else
    {
        _231 = false;
    }
    bool _236 = false;
    if (_231)
    {
        int _234 = _194;
        _236 = return_with_fallthrough_bi(_234);
    }
    else
    {
        _236 = false;
    }
    bool _241 = false;
    if (_236)
    {
        int _239 = _194;
        _241 = fallthrough_ends_in_break_bi(_239);
    }
    else
    {
        _241 = false;
    }
    bool _246 = false;
    if (_241)
    {
        int _244 = _194;
        _246 = fallthrough_to_default_with_break_bi(_244);
    }
    else
    {
        _246 = false;
    }
    bool _251 = false;
    if (_246)
    {
        int _249 = _194;
        _251 = fallthrough_to_default_with_return_bi(_249);
    }
    else
    {
        _251 = false;
    }
    bool _256 = false;
    if (_251)
    {
        int _254 = _194;
        _256 = fallthrough_with_loop_break_bi(_254);
    }
    else
    {
        _256 = false;
    }
    bool _261 = false;
    if (_256)
    {
        int _259 = _194;
        _261 = fallthrough_with_loop_continue_bi(_259);
    }
    else
    {
        _261 = false;
    }
    float4 _262 = 0.0f.xxxx;
    if (_261)
    {
        _262 = _21_colorGreen;
    }
    else
    {
        _262 = _21_colorRed;
    }
    return _262;
}

void frag_main()
{
    float2 _31 = 0.0f.xx;
    sk_FragColor = main(_31);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
