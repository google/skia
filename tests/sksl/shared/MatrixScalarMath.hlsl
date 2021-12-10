cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_bifffff22(int _30, float _31, float _32, float _33, float _34, float2x2 _35)
{
    float one = _11_colorRed.x;
    float2 _52 = float2(_31 * _11_colorRed.x, _32 * _11_colorRed.x);
    float2 _53 = float2(_33 * _11_colorRed.x, _34 * _11_colorRed.x);
    float2x2 m2 = float2x2(_52, _53);
    switch (_30)
    {
        case 1:
        {
            m2 = float2x2(_52 + 1.0f.xx, _53 + 1.0f.xx);
            break;
        }
        case 2:
        {
            m2 = float2x2(m2[0] - 1.0f.xx, m2[1] - 1.0f.xx);
            break;
        }
        case 3:
        {
            m2 = m2 * 2.0f;
            break;
        }
        case 4:
        {
            m2 = float2x2(m2[0] / 2.0f.xx, m2[1] / 2.0f.xx);
            break;
        }
    }
    bool _102 = false;
    if (m2[0].x == _35[0].x)
    {
        _102 = m2[0].y == _35[0].y;
    }
    else
    {
        _102 = false;
    }
    bool _112 = false;
    if (_102)
    {
        _112 = m2[1].x == _35[1].x;
    }
    else
    {
        _112 = false;
    }
    bool _122 = false;
    if (_112)
    {
        _122 = m2[1].y == _35[1].y;
    }
    else
    {
        _122 = false;
    }
    return _122;
}

float4 main(float2 _124)
{
    float f1 = _11_colorGreen.y;
    float _134 = 2.0f * _11_colorGreen.y;
    float f2 = _134;
    float _140 = 3.0f * _11_colorGreen.y;
    float f3 = _140;
    float _146 = 4.0f * _11_colorGreen.y;
    float f4 = _146;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_0_expected = float2x2(float2(_11_colorGreen.y + 1.0f, _134 + 1.0f), float2(_140 + 1.0f, _146 + 1.0f));
    float _RESERVED_IDENTIFIER_FIXUP_1_one = _11_colorRed.x;
    float2 _164 = float2(_11_colorGreen.y * _11_colorRed.x, _134 * _11_colorRed.x);
    float2 _165 = float2(_140 * _11_colorRed.x, _146 * _11_colorRed.x);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_164, _165);
    _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_164 + 1.0f.xx, _165 + 1.0f.xx);
    bool _186 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_2_m2[0].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].x)
    {
        _186 = _RESERVED_IDENTIFIER_FIXUP_2_m2[0].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].y;
    }
    else
    {
        _186 = false;
    }
    bool _196 = false;
    if (_186)
    {
        _196 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].x;
    }
    else
    {
        _196 = false;
    }
    bool _206 = false;
    if (_196)
    {
        _206 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].y;
    }
    else
    {
        _206 = false;
    }
    bool _224 = false;
    if (_206)
    {
        int _210 = 2;
        float _211 = _11_colorGreen.y;
        float _212 = _134;
        float _213 = _140;
        float _214 = _146;
        float2x2 _222 = float2x2(float2(_11_colorGreen.y - 1.0f, _134 - 1.0f), float2(_140 - 1.0f, _146 - 1.0f));
        _224 = test_bifffff22(_210, _211, _212, _213, _214, _222);
    }
    else
    {
        _224 = false;
    }
    bool _242 = false;
    if (_224)
    {
        int _228 = 3;
        float _229 = _11_colorGreen.y;
        float _230 = _134;
        float _231 = _140;
        float _232 = _146;
        float2x2 _240 = float2x2(float2(_11_colorGreen.y * 2.0f, _134 * 2.0f), float2(_140 * 2.0f, _146 * 2.0f));
        _242 = test_bifffff22(_228, _229, _230, _231, _232, _240);
    }
    else
    {
        _242 = false;
    }
    bool _261 = false;
    if (_242)
    {
        int _246 = 4;
        float _247 = _11_colorGreen.y;
        float _248 = _134;
        float _249 = _140;
        float _250 = _146;
        float2x2 _259 = float2x2(float2(_11_colorGreen.y * 0.5f, _134 * 0.5f), float2(_140 * 0.5f, _146 * 0.5f));
        _261 = test_bifffff22(_246, _247, _248, _249, _250, _259);
    }
    else
    {
        _261 = false;
    }
    float4 _262 = 0.0f.xxxx;
    if (_261)
    {
        _262 = _11_colorGreen;
    }
    else
    {
        _262 = _11_colorRed;
    }
    return _262;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
