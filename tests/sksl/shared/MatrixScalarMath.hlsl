cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _13_colorGreen : packoffset(c0);
    float4 _13_colorRed : packoffset(c1);
    float4 _13_testInputs : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_bifffff22(int _32, float _33, float _34, float _35, float _36, float2x2 _37)
{
    float one = _13_colorRed.x;
    float2 _54 = float2(_33 * _13_colorRed.x, _34 * _13_colorRed.x);
    float2 _55 = float2(_35 * _13_colorRed.x, _36 * _13_colorRed.x);
    float2x2 m2 = float2x2(_54, _55);
    switch (_32)
    {
        case 1:
        {
            m2 = float2x2(1.0f.xx + _54, 1.0f.xx + _55);
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
    bool _99 = false;
    if (m2[0].x == _37[0].x)
    {
        _99 = m2[0].y == _37[0].y;
    }
    else
    {
        _99 = false;
    }
    bool _109 = false;
    if (_99)
    {
        _109 = m2[1].x == _37[1].x;
    }
    else
    {
        _109 = false;
    }
    bool _119 = false;
    if (_109)
    {
        _119 = m2[1].y == _37[1].y;
    }
    else
    {
        _119 = false;
    }
    return _119;
}

bool divisionTest_b()
{
    float _127 = _13_colorRed.x * 10.0f;
    float ten = _127;
    float2 _129 = _127.xx;
    float2x2 _130 = float2x2(_129, _129);
    float2x2 mat = _130;
    float2x2 _137 = _130 * (1.0f / _13_testInputs.x);
    float2x2 div = _137;
    float2x2 _142 = _130 * (1.0f / _13_testInputs.x);
    mat = _142;
    float4 _145 = abs(float4(_137[0].x, _137[0].y, _137[1].x, _137[1].y) + 8.0f.xxxx);
    bool _168 = false;
    if (all(bool4(_145.x < 0.00999999977648258209228515625f.xxxx.x, _145.y < 0.00999999977648258209228515625f.xxxx.y, _145.z < 0.00999999977648258209228515625f.xxxx.z, _145.w < 0.00999999977648258209228515625f.xxxx.w)))
    {
        float4 _161 = abs(float4(_142[0].x, _142[0].y, _142[1].x, _142[1].y) + 8.0f.xxxx);
        _168 = all(bool4(_161.x < 0.00999999977648258209228515625f.xxxx.x, _161.y < 0.00999999977648258209228515625f.xxxx.y, _161.z < 0.00999999977648258209228515625f.xxxx.z, _161.w < 0.00999999977648258209228515625f.xxxx.w));
    }
    else
    {
        _168 = false;
    }
    return _168;
}

float4 main(float2 _170)
{
    float f1 = _13_colorGreen.y;
    float _180 = 2.0f * _13_colorGreen.y;
    float f2 = _180;
    float _186 = 3.0f * _13_colorGreen.y;
    float f3 = _186;
    float _192 = 4.0f * _13_colorGreen.y;
    float f4 = _192;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_0_expected = float2x2(float2(_13_colorGreen.y + 1.0f, _180 + 1.0f), float2(_186 + 1.0f, _192 + 1.0f));
    float _RESERVED_IDENTIFIER_FIXUP_1_one = _13_colorRed.x;
    float2 _210 = float2(_13_colorGreen.y * _13_colorRed.x, _180 * _13_colorRed.x);
    float2 _211 = float2(_186 * _13_colorRed.x, _192 * _13_colorRed.x);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_210, _211);
    _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(1.0f.xx + _210, 1.0f.xx + _211);
    bool _232 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_2_m2[0].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].x)
    {
        _232 = _RESERVED_IDENTIFIER_FIXUP_2_m2[0].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].y;
    }
    else
    {
        _232 = false;
    }
    bool _242 = false;
    if (_232)
    {
        _242 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].x;
    }
    else
    {
        _242 = false;
    }
    bool _252 = false;
    if (_242)
    {
        _252 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].y;
    }
    else
    {
        _252 = false;
    }
    bool _269 = false;
    if (_252)
    {
        int _255 = 2;
        float _256 = _13_colorGreen.y;
        float _257 = _180;
        float _258 = _186;
        float _259 = _192;
        float2x2 _267 = float2x2(float2(_13_colorGreen.y - 1.0f, _180 - 1.0f), float2(_186 - 1.0f, _192 - 1.0f));
        _269 = test_bifffff22(_255, _256, _257, _258, _259, _267);
    }
    else
    {
        _269 = false;
    }
    bool _287 = false;
    if (_269)
    {
        int _273 = 3;
        float _274 = _13_colorGreen.y;
        float _275 = _180;
        float _276 = _186;
        float _277 = _192;
        float2x2 _285 = float2x2(float2(_13_colorGreen.y * 2.0f, _180 * 2.0f), float2(_186 * 2.0f, _192 * 2.0f));
        _287 = test_bifffff22(_273, _274, _275, _276, _277, _285);
    }
    else
    {
        _287 = false;
    }
    bool _305 = false;
    if (_287)
    {
        int _291 = 4;
        float _292 = _13_colorGreen.y;
        float _293 = _180;
        float _294 = _186;
        float _295 = _192;
        float2x2 _303 = float2x2(float2(_13_colorGreen.y * 0.5f, _180 * 0.5f), float2(_186 * 0.5f, _192 * 0.5f));
        _305 = test_bifffff22(_291, _292, _293, _294, _295, _303);
    }
    else
    {
        _305 = false;
    }
    bool _309 = false;
    if (_305)
    {
        _309 = divisionTest_b();
    }
    else
    {
        _309 = false;
    }
    float4 _310 = 0.0f.xxxx;
    if (_309)
    {
        _310 = _13_colorGreen;
    }
    else
    {
        _310 = _13_colorRed;
    }
    return _310;
}

void frag_main()
{
    float2 _23 = 0.0f.xx;
    sk_FragColor = main(_23);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
