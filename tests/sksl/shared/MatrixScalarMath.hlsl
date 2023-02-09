cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
    float4 _12_testInputs : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_bifffff22(int _31, float _32, float _33, float _34, float _35, float2x2 _36)
{
    float one = _12_colorRed.x;
    float2 _53 = float2(_32 * _12_colorRed.x, _33 * _12_colorRed.x);
    float2 _54 = float2(_34 * _12_colorRed.x, _35 * _12_colorRed.x);
    float2x2 m2 = float2x2(_53, _54);
    switch (_31)
    {
        case 1:
        {
            m2 = float2x2(_53 + 1.0f.xx, _54 + 1.0f.xx);
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
    bool _98 = false;
    if (m2[0].x == _36[0].x)
    {
        _98 = m2[0].y == _36[0].y;
    }
    else
    {
        _98 = false;
    }
    bool _108 = false;
    if (_98)
    {
        _108 = m2[1].x == _36[1].x;
    }
    else
    {
        _108 = false;
    }
    bool _118 = false;
    if (_108)
    {
        _118 = m2[1].y == _36[1].y;
    }
    else
    {
        _118 = false;
    }
    return _118;
}

bool divisionTest_b()
{
    float _126 = _12_colorRed.x * 10.0f;
    float ten = _126;
    float2 _128 = _126.xx;
    float2x2 _129 = float2x2(_128, _128);
    float2x2 mat = _129;
    float2x2 _136 = _129 * (1.0f / _12_testInputs.x);
    float2x2 div = _136;
    float2x2 _141 = _129 * (1.0f / _12_testInputs.x);
    mat = _141;
    float2 _146 = _136[0];
    float2 _149 = _136[1];
    bool _162 = false;
    if (all(bool2(_146.x == (-8.0f).xx.x, _146.y == (-8.0f).xx.y)) && all(bool2(_149.x == (-8.0f).xx.x, _149.y == (-8.0f).xx.y)))
    {
        float2 _155 = _141[0];
        float2 _158 = _141[1];
        _162 = all(bool2(_155.x == (-8.0f).xx.x, _155.y == (-8.0f).xx.y)) && all(bool2(_158.x == (-8.0f).xx.x, _158.y == (-8.0f).xx.y));
    }
    else
    {
        _162 = false;
    }
    return _162;
}

float4 main(float2 _164)
{
    float f1 = _12_colorGreen.y;
    float _174 = 2.0f * _12_colorGreen.y;
    float f2 = _174;
    float _180 = 3.0f * _12_colorGreen.y;
    float f3 = _180;
    float _186 = 4.0f * _12_colorGreen.y;
    float f4 = _186;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_0_expected = float2x2(float2(_12_colorGreen.y + 1.0f, _174 + 1.0f), float2(_180 + 1.0f, _186 + 1.0f));
    float _RESERVED_IDENTIFIER_FIXUP_1_one = _12_colorRed.x;
    float2 _204 = float2(_12_colorGreen.y * _12_colorRed.x, _174 * _12_colorRed.x);
    float2 _205 = float2(_180 * _12_colorRed.x, _186 * _12_colorRed.x);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_204, _205);
    _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_204 + 1.0f.xx, _205 + 1.0f.xx);
    bool _226 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_2_m2[0].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].x)
    {
        _226 = _RESERVED_IDENTIFIER_FIXUP_2_m2[0].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].y;
    }
    else
    {
        _226 = false;
    }
    bool _236 = false;
    if (_226)
    {
        _236 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].x;
    }
    else
    {
        _236 = false;
    }
    bool _246 = false;
    if (_236)
    {
        _246 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].y;
    }
    else
    {
        _246 = false;
    }
    bool _263 = false;
    if (_246)
    {
        int _249 = 2;
        float _250 = _12_colorGreen.y;
        float _251 = _174;
        float _252 = _180;
        float _253 = _186;
        float2x2 _261 = float2x2(float2(_12_colorGreen.y - 1.0f, _174 - 1.0f), float2(_180 - 1.0f, _186 - 1.0f));
        _263 = test_bifffff22(_249, _250, _251, _252, _253, _261);
    }
    else
    {
        _263 = false;
    }
    bool _281 = false;
    if (_263)
    {
        int _267 = 3;
        float _268 = _12_colorGreen.y;
        float _269 = _174;
        float _270 = _180;
        float _271 = _186;
        float2x2 _279 = float2x2(float2(_12_colorGreen.y * 2.0f, _174 * 2.0f), float2(_180 * 2.0f, _186 * 2.0f));
        _281 = test_bifffff22(_267, _268, _269, _270, _271, _279);
    }
    else
    {
        _281 = false;
    }
    bool _299 = false;
    if (_281)
    {
        int _285 = 4;
        float _286 = _12_colorGreen.y;
        float _287 = _174;
        float _288 = _180;
        float _289 = _186;
        float2x2 _297 = float2x2(float2(_12_colorGreen.y * 0.5f, _174 * 0.5f), float2(_180 * 0.5f, _186 * 0.5f));
        _299 = test_bifffff22(_285, _286, _287, _288, _289, _297);
    }
    else
    {
        _299 = false;
    }
    bool _303 = false;
    if (_299)
    {
        _303 = divisionTest_b();
    }
    else
    {
        _303 = false;
    }
    float4 _304 = 0.0f.xxxx;
    if (_303)
    {
        _304 = _12_colorGreen;
    }
    else
    {
        _304 = _12_colorRed;
    }
    return _304;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
