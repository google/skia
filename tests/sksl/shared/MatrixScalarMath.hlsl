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
            m2 = m2 * 0.5f;
            break;
        }
    }
    bool _97 = false;
    if (m2[0].x == _35[0].x)
    {
        _97 = m2[0].y == _35[0].y;
    }
    else
    {
        _97 = false;
    }
    bool _107 = false;
    if (_97)
    {
        _107 = m2[1].x == _35[1].x;
    }
    else
    {
        _107 = false;
    }
    bool _117 = false;
    if (_107)
    {
        _117 = m2[1].y == _35[1].y;
    }
    else
    {
        _117 = false;
    }
    return _117;
}

float4 main(float2 _119)
{
    float f1 = _11_colorGreen.y;
    float _129 = 2.0f * _11_colorGreen.y;
    float f2 = _129;
    float _135 = 3.0f * _11_colorGreen.y;
    float f3 = _135;
    float _141 = 4.0f * _11_colorGreen.y;
    float f4 = _141;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_0_expected = float2x2(float2(_11_colorGreen.y + 1.0f, _129 + 1.0f), float2(_135 + 1.0f, _141 + 1.0f));
    float _RESERVED_IDENTIFIER_FIXUP_1_one = _11_colorRed.x;
    float2 _159 = float2(_11_colorGreen.y * _11_colorRed.x, _129 * _11_colorRed.x);
    float2 _160 = float2(_135 * _11_colorRed.x, _141 * _11_colorRed.x);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_159, _160);
    _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_159 + 1.0f.xx, _160 + 1.0f.xx);
    bool _181 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_2_m2[0].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].x)
    {
        _181 = _RESERVED_IDENTIFIER_FIXUP_2_m2[0].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].y;
    }
    else
    {
        _181 = false;
    }
    bool _191 = false;
    if (_181)
    {
        _191 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].x;
    }
    else
    {
        _191 = false;
    }
    bool _201 = false;
    if (_191)
    {
        _201 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].y;
    }
    else
    {
        _201 = false;
    }
    bool _219 = false;
    if (_201)
    {
        int _205 = 2;
        float _206 = _11_colorGreen.y;
        float _207 = _129;
        float _208 = _135;
        float _209 = _141;
        float2x2 _217 = float2x2(float2(_11_colorGreen.y - 1.0f, _129 - 1.0f), float2(_135 - 1.0f, _141 - 1.0f));
        _219 = test_bifffff22(_205, _206, _207, _208, _209, _217);
    }
    else
    {
        _219 = false;
    }
    bool _237 = false;
    if (_219)
    {
        int _223 = 3;
        float _224 = _11_colorGreen.y;
        float _225 = _129;
        float _226 = _135;
        float _227 = _141;
        float2x2 _235 = float2x2(float2(_11_colorGreen.y * 2.0f, _129 * 2.0f), float2(_135 * 2.0f, _141 * 2.0f));
        _237 = test_bifffff22(_223, _224, _225, _226, _227, _235);
    }
    else
    {
        _237 = false;
    }
    bool _255 = false;
    if (_237)
    {
        int _241 = 4;
        float _242 = _11_colorGreen.y;
        float _243 = _129;
        float _244 = _135;
        float _245 = _141;
        float2x2 _253 = float2x2(float2(_11_colorGreen.y * 0.5f, _129 * 0.5f), float2(_135 * 0.5f, _141 * 0.5f));
        _255 = test_bifffff22(_241, _242, _243, _244, _245, _253);
    }
    else
    {
        _255 = false;
    }
    float4 _256 = 0.0f.xxxx;
    if (_255)
    {
        _256 = _11_colorGreen;
    }
    else
    {
        _256 = _11_colorRed;
    }
    return _256;
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
