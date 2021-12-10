cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_ivec_b()
{
    int one = 1;
    int two = 2;
    bool ok = true;
    bool _56 = false;
    if (ok)
    {
        int2 _40 = -int2(-one, one + one);
        int2 _48 = -int2(one - two, 2);
        _56 = all(bool2(_40.x == _48.x, _40.y == _48.y));
    }
    else
    {
        _56 = false;
    }
    ok = _56;
    return ok;
}

bool test_mat_b()
{
    bool ok = true;
    return ok;
}

float4 main(float2 _62)
{
    float _RESERVED_IDENTIFIER_FIXUP_0_one = 1.0f;
    float _RESERVED_IDENTIFIER_FIXUP_1_two = 2.0f;
    bool _RESERVED_IDENTIFIER_FIXUP_4_ok = true;
    bool _89 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_4_ok)
    {
        float4 _73 = -_RESERVED_IDENTIFIER_FIXUP_1_two.xxxx;
        float4 _85 = float4(-_RESERVED_IDENTIFIER_FIXUP_1_two, (-_RESERVED_IDENTIFIER_FIXUP_1_two).xxx);
        _89 = all(bool4(_73.x == _85.x, _73.y == _85.y, _73.z == _85.z, _73.w == _85.w));
    }
    else
    {
        _89 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_4_ok = _89;
    bool _103 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_4_ok)
    {
        float2 _95 = -float2(_RESERVED_IDENTIFIER_FIXUP_0_one - _RESERVED_IDENTIFIER_FIXUP_1_two, _RESERVED_IDENTIFIER_FIXUP_1_two);
        _103 = all(bool2(float2(1.0f, -2.0f).x == _95.x, float2(1.0f, -2.0f).y == _95.y));
    }
    else
    {
        _103 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_4_ok = _103;
    bool _108 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_4_ok)
    {
        _108 = test_ivec_b();
    }
    else
    {
        _108 = false;
    }
    bool _112 = false;
    if (_108)
    {
        _112 = test_mat_b();
    }
    else
    {
        _112 = false;
    }
    float4 _113 = 0.0f.xxxx;
    if (_112)
    {
        _113 = _12_colorGreen;
    }
    else
    {
        _113 = _12_colorRed;
    }
    return _113;
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
