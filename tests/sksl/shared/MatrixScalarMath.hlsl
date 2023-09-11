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
            m2 = float2x2(1.0f.xx + _53, 1.0f.xx + _54);
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
    float4 _144 = abs(float4(_136[0].x, _136[0].y, _136[1].x, _136[1].y) + 8.0f.xxxx);
    bool _167 = false;
    if (all(bool4(_144.x < 0.00999999977648258209228515625f.xxxx.x, _144.y < 0.00999999977648258209228515625f.xxxx.y, _144.z < 0.00999999977648258209228515625f.xxxx.z, _144.w < 0.00999999977648258209228515625f.xxxx.w)))
    {
        float4 _160 = abs(float4(_141[0].x, _141[0].y, _141[1].x, _141[1].y) + 8.0f.xxxx);
        _167 = all(bool4(_160.x < 0.00999999977648258209228515625f.xxxx.x, _160.y < 0.00999999977648258209228515625f.xxxx.y, _160.z < 0.00999999977648258209228515625f.xxxx.z, _160.w < 0.00999999977648258209228515625f.xxxx.w));
    }
    else
    {
        _167 = false;
    }
    return _167;
}

float4 main(float2 _169)
{
    float f1 = _12_colorGreen.y;
    float _179 = 2.0f * _12_colorGreen.y;
    float f2 = _179;
    float _185 = 3.0f * _12_colorGreen.y;
    float f3 = _185;
    float _191 = 4.0f * _12_colorGreen.y;
    float f4 = _191;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_0_expected = float2x2(float2(_12_colorGreen.y + 1.0f, _179 + 1.0f), float2(_185 + 1.0f, _191 + 1.0f));
    float _RESERVED_IDENTIFIER_FIXUP_1_one = _12_colorRed.x;
    float2 _209 = float2(_12_colorGreen.y * _12_colorRed.x, _179 * _12_colorRed.x);
    float2 _210 = float2(_185 * _12_colorRed.x, _191 * _12_colorRed.x);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_209, _210);
    _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(1.0f.xx + _209, 1.0f.xx + _210);
    bool _231 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_2_m2[0].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].x)
    {
        _231 = _RESERVED_IDENTIFIER_FIXUP_2_m2[0].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].y;
    }
    else
    {
        _231 = false;
    }
    bool _241 = false;
    if (_231)
    {
        _241 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].x;
    }
    else
    {
        _241 = false;
    }
    bool _251 = false;
    if (_241)
    {
        _251 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].y;
    }
    else
    {
        _251 = false;
    }
    bool _268 = false;
    if (_251)
    {
        int _254 = 2;
        float _255 = _12_colorGreen.y;
        float _256 = _179;
        float _257 = _185;
        float _258 = _191;
        float2x2 _266 = float2x2(float2(_12_colorGreen.y - 1.0f, _179 - 1.0f), float2(_185 - 1.0f, _191 - 1.0f));
        _268 = test_bifffff22(_254, _255, _256, _257, _258, _266);
    }
    else
    {
        _268 = false;
    }
    bool _286 = false;
    if (_268)
    {
        int _272 = 3;
        float _273 = _12_colorGreen.y;
        float _274 = _179;
        float _275 = _185;
        float _276 = _191;
        float2x2 _284 = float2x2(float2(_12_colorGreen.y * 2.0f, _179 * 2.0f), float2(_185 * 2.0f, _191 * 2.0f));
        _286 = test_bifffff22(_272, _273, _274, _275, _276, _284);
    }
    else
    {
        _286 = false;
    }
    bool _304 = false;
    if (_286)
    {
        int _290 = 4;
        float _291 = _12_colorGreen.y;
        float _292 = _179;
        float _293 = _185;
        float _294 = _191;
        float2x2 _302 = float2x2(float2(_12_colorGreen.y * 0.5f, _179 * 0.5f), float2(_185 * 0.5f, _191 * 0.5f));
        _304 = test_bifffff22(_290, _291, _292, _293, _294, _302);
    }
    else
    {
        _304 = false;
    }
    bool _308 = false;
    if (_304)
    {
        _308 = divisionTest_b();
    }
    else
    {
        _308 = false;
    }
    float4 _309 = 0.0f.xxxx;
    if (_308)
    {
        _309 = _12_colorGreen;
    }
    else
    {
        _309 = _12_colorRed;
    }
    return _309;
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
