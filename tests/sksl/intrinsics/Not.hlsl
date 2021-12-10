cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_inputH4 : packoffset(c0);
    float4 _10_expectedH4 : packoffset(c1);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool4 _42 = bool4(_10_inputH4.x != 0.0f, _10_inputH4.y != 0.0f, _10_inputH4.z != 0.0f, _10_inputH4.w != 0.0f);
    bool4 inputVal = _42;
    bool4 _55 = bool4(_10_expectedH4.x != 0.0f, _10_expectedH4.y != 0.0f, _10_expectedH4.z != 0.0f, _10_expectedH4.w != 0.0f);
    bool4 expected = _55;
    bool2 _58 = _42.xy;
    bool2 _57 = bool2(!_58.x, !_58.y);
    bool2 _60 = _55.xy;
    bool _71 = false;
    if (all(bool2(_57.x == _60.x, _57.y == _60.y)))
    {
        bool3 _66 = _42.xyz;
        bool3 _65 = bool3(!_66.x, !_66.y, !_66.z);
        bool3 _68 = _55.xyz;
        _71 = all(bool3(_65.x == _68.x, _65.y == _68.y, _65.z == _68.z));
    }
    else
    {
        _71 = false;
    }
    bool _77 = false;
    if (_71)
    {
        bool4 _74 = bool4(!_42.x, !_42.y, !_42.z, !_42.w);
        _77 = all(bool4(_74.x == _55.x, _74.y == _55.y, _74.z == _55.z, _74.w == _55.w));
    }
    else
    {
        _77 = false;
    }
    bool _85 = false;
    if (_77)
    {
        bool2 _82 = _55.xy;
        _85 = all(bool2(bool2(false, true).x == _82.x, bool2(false, true).y == _82.y));
    }
    else
    {
        _85 = false;
    }
    bool _92 = false;
    if (_85)
    {
        bool3 _89 = _55.xyz;
        _92 = all(bool3(bool3(false, true, false).x == _89.x, bool3(false, true, false).y == _89.y, bool3(false, true, false).z == _89.z));
    }
    else
    {
        _92 = false;
    }
    bool _98 = false;
    if (_92)
    {
        _98 = all(bool4(bool4(false, true, false, true).x == _55.x, bool4(false, true, false, true).y == _55.y, bool4(false, true, false, true).z == _55.z, bool4(false, true, false, true).w == _55.w));
    }
    else
    {
        _98 = false;
    }
    float4 _99 = 0.0f.xxxx;
    if (_98)
    {
        _99 = _10_colorGreen;
    }
    else
    {
        _99 = _10_colorRed;
    }
    return _99;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
