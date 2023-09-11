cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _9_colorGreen : packoffset(c0);
    float4 _9_colorRed : packoffset(c1);
    float4 _9_testInputs : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_bifffff22(int _29, float _30, float _31, float _32, float _33, float2x2 _34)
{
    float one = _9_colorRed.x;
    float2 _51 = float2(_30 * _9_colorRed.x, _31 * _9_colorRed.x);
    float2 _52 = float2(_32 * _9_colorRed.x, _33 * _9_colorRed.x);
    float2x2 m2 = float2x2(_51, _52);
    switch (_29)
    {
        case 1:
        {
            m2 = float2x2(1.0f.xx + _51, 1.0f.xx + _52);
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
            m2 = m2 * 0.5f;
            break;
        }
    }
    bool _96 = false;
    if (m2[0].x == _34[0].x)
    {
        _96 = m2[0].y == _34[0].y;
    }
    else
    {
        _96 = false;
    }
    bool _106 = false;
    if (_96)
    {
        _106 = m2[1].x == _34[1].x;
    }
    else
    {
        _106 = false;
    }
    bool _116 = false;
    if (_106)
    {
        _116 = m2[1].y == _34[1].y;
    }
    else
    {
        _116 = false;
    }
    return _116;
}

bool divisionTest_b()
{
    float _124 = _9_colorRed.x * 10.0f;
    float ten = _124;
    float2 _126 = _124.xx;
    float2x2 _127 = float2x2(_126, _126);
    float2x2 mat = _127;
    float2x2 _134 = _127 * (1.0f / _9_testInputs.x);
    float2x2 div = _134;
    float2x2 _139 = _127 * (1.0f / _9_testInputs.x);
    mat = _139;
    float4 _142 = abs(float4(_134[0].x, _134[0].y, _134[1].x, _134[1].y) + 8.0f.xxxx);
    bool _165 = false;
    if (all(bool4(_142.x < 0.00999999977648258209228515625f.xxxx.x, _142.y < 0.00999999977648258209228515625f.xxxx.y, _142.z < 0.00999999977648258209228515625f.xxxx.z, _142.w < 0.00999999977648258209228515625f.xxxx.w)))
    {
        float4 _158 = abs(float4(_139[0].x, _139[0].y, _139[1].x, _139[1].y) + 8.0f.xxxx);
        _165 = all(bool4(_158.x < 0.00999999977648258209228515625f.xxxx.x, _158.y < 0.00999999977648258209228515625f.xxxx.y, _158.z < 0.00999999977648258209228515625f.xxxx.z, _158.w < 0.00999999977648258209228515625f.xxxx.w));
    }
    else
    {
        _165 = false;
    }
    return _165;
}

float4 main(float2 _167)
{
    float f1 = _9_colorGreen.y;
    float _177 = 2.0f * _9_colorGreen.y;
    float f2 = _177;
    float _183 = 3.0f * _9_colorGreen.y;
    float f3 = _183;
    float _189 = 4.0f * _9_colorGreen.y;
    float f4 = _189;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_0_expected = float2x2(float2(_9_colorGreen.y + 1.0f, _177 + 1.0f), float2(_183 + 1.0f, _189 + 1.0f));
    float _RESERVED_IDENTIFIER_FIXUP_1_one = _9_colorRed.x;
    float2 _207 = float2(_9_colorGreen.y * _9_colorRed.x, _177 * _9_colorRed.x);
    float2 _208 = float2(_183 * _9_colorRed.x, _189 * _9_colorRed.x);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_207, _208);
    _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(1.0f.xx + _207, 1.0f.xx + _208);
    bool _229 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_2_m2[0].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].x)
    {
        _229 = _RESERVED_IDENTIFIER_FIXUP_2_m2[0].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].y;
    }
    else
    {
        _229 = false;
    }
    bool _239 = false;
    if (_229)
    {
        _239 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].x;
    }
    else
    {
        _239 = false;
    }
    bool _249 = false;
    if (_239)
    {
        _249 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].y;
    }
    else
    {
        _249 = false;
    }
    bool _266 = false;
    if (_249)
    {
        int _252 = 2;
        float _253 = _9_colorGreen.y;
        float _254 = _177;
        float _255 = _183;
        float _256 = _189;
        float2x2 _264 = float2x2(float2(_9_colorGreen.y - 1.0f, _177 - 1.0f), float2(_183 - 1.0f, _189 - 1.0f));
        _266 = test_bifffff22(_252, _253, _254, _255, _256, _264);
    }
    else
    {
        _266 = false;
    }
    bool _284 = false;
    if (_266)
    {
        int _270 = 3;
        float _271 = _9_colorGreen.y;
        float _272 = _177;
        float _273 = _183;
        float _274 = _189;
        float2x2 _282 = float2x2(float2(_9_colorGreen.y * 2.0f, _177 * 2.0f), float2(_183 * 2.0f, _189 * 2.0f));
        _284 = test_bifffff22(_270, _271, _272, _273, _274, _282);
    }
    else
    {
        _284 = false;
    }
    bool _302 = false;
    if (_284)
    {
        int _288 = 4;
        float _289 = _9_colorGreen.y;
        float _290 = _177;
        float _291 = _183;
        float _292 = _189;
        float2x2 _300 = float2x2(float2(_9_colorGreen.y * 0.5f, _177 * 0.5f), float2(_183 * 0.5f, _189 * 0.5f));
        _302 = test_bifffff22(_288, _289, _290, _291, _292, _300);
    }
    else
    {
        _302 = false;
    }
    bool _306 = false;
    if (_302)
    {
        _306 = divisionTest_b();
    }
    else
    {
        _306 = false;
    }
    float4 _307 = 0.0f.xxxx;
    if (_306)
    {
        _307 = _9_colorGreen;
    }
    else
    {
        _307 = _9_colorRed;
    }
    return _307;
}

void frag_main()
{
    float2 _19 = 0.0f.xx;
    sk_FragColor = main(_19);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
