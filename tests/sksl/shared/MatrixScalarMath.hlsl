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
    float2x2 mat = float2x2(_128, _128);
    float2 _135 = _12_testInputs.x.xx;
    float2 _137 = _128 / _135;
    float2 _138 = _128 / _135;
    float2x2 div = float2x2(_137, _138);
    float2 _143 = _12_testInputs.x.xx;
    float2 _145 = _128 / _143;
    float2 _146 = _128 / _143;
    mat = float2x2(_145, _146);
    bool _164 = false;
    if (all(bool2(_137.x == (-8.0f).xx.x, _137.y == (-8.0f).xx.y)) && all(bool2(_138.x == (-8.0f).xx.x, _138.y == (-8.0f).xx.y)))
    {
        _164 = all(bool2(_145.x == (-8.0f).xx.x, _145.y == (-8.0f).xx.y)) && all(bool2(_146.x == (-8.0f).xx.x, _146.y == (-8.0f).xx.y));
    }
    else
    {
        _164 = false;
    }
    return _164;
}

float4 main(float2 _166)
{
    float f1 = _12_colorGreen.y;
    float _176 = 2.0f * _12_colorGreen.y;
    float f2 = _176;
    float _182 = 3.0f * _12_colorGreen.y;
    float f3 = _182;
    float _188 = 4.0f * _12_colorGreen.y;
    float f4 = _188;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_0_expected = float2x2(float2(_12_colorGreen.y + 1.0f, _176 + 1.0f), float2(_182 + 1.0f, _188 + 1.0f));
    float _RESERVED_IDENTIFIER_FIXUP_1_one = _12_colorRed.x;
    float2 _206 = float2(_12_colorGreen.y * _12_colorRed.x, _176 * _12_colorRed.x);
    float2 _207 = float2(_182 * _12_colorRed.x, _188 * _12_colorRed.x);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_206, _207);
    _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_206 + 1.0f.xx, _207 + 1.0f.xx);
    bool _228 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_2_m2[0].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].x)
    {
        _228 = _RESERVED_IDENTIFIER_FIXUP_2_m2[0].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].y;
    }
    else
    {
        _228 = false;
    }
    bool _238 = false;
    if (_228)
    {
        _238 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].x;
    }
    else
    {
        _238 = false;
    }
    bool _248 = false;
    if (_238)
    {
        _248 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].y;
    }
    else
    {
        _248 = false;
    }
    bool _265 = false;
    if (_248)
    {
        int _251 = 2;
        float _252 = _12_colorGreen.y;
        float _253 = _176;
        float _254 = _182;
        float _255 = _188;
        float2x2 _263 = float2x2(float2(_12_colorGreen.y - 1.0f, _176 - 1.0f), float2(_182 - 1.0f, _188 - 1.0f));
        _265 = test_bifffff22(_251, _252, _253, _254, _255, _263);
    }
    else
    {
        _265 = false;
    }
    bool _283 = false;
    if (_265)
    {
        int _269 = 3;
        float _270 = _12_colorGreen.y;
        float _271 = _176;
        float _272 = _182;
        float _273 = _188;
        float2x2 _281 = float2x2(float2(_12_colorGreen.y * 2.0f, _176 * 2.0f), float2(_182 * 2.0f, _188 * 2.0f));
        _283 = test_bifffff22(_269, _270, _271, _272, _273, _281);
    }
    else
    {
        _283 = false;
    }
    bool _301 = false;
    if (_283)
    {
        int _287 = 4;
        float _288 = _12_colorGreen.y;
        float _289 = _176;
        float _290 = _182;
        float _291 = _188;
        float2x2 _299 = float2x2(float2(_12_colorGreen.y * 0.5f, _176 * 0.5f), float2(_182 * 0.5f, _188 * 0.5f));
        _301 = test_bifffff22(_287, _288, _289, _290, _291, _299);
    }
    else
    {
        _301 = false;
    }
    bool _305 = false;
    if (_301)
    {
        _305 = divisionTest_b();
    }
    else
    {
        _305 = false;
    }
    float4 _306 = 0.0f.xxxx;
    if (_305)
    {
        _306 = _12_colorGreen;
    }
    else
    {
        _306 = _12_colorRed;
    }
    return _306;
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
