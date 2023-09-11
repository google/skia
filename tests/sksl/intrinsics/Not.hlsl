cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    bool4 _40 = bool4(_7_colorGreen.x != 0.0f, _7_colorGreen.y != 0.0f, _7_colorGreen.z != 0.0f, _7_colorGreen.w != 0.0f);
    bool4 inputVal = _40;
    bool4 expected = bool4(true, false, true, false);
    bool2 _46 = _40.xy;
    bool2 _45 = bool2(!_46.x, !_46.y);
    bool _59 = false;
    if (all(bool2(_45.x == bool4(true, false, true, false).xy.x, _45.y == bool4(true, false, true, false).xy.y)))
    {
        bool3 _54 = _40.xyz;
        bool3 _53 = bool3(!_54.x, !_54.y, !_54.z);
        _59 = all(bool3(_53.x == bool4(true, false, true, false).xyz.x, _53.y == bool4(true, false, true, false).xyz.y, _53.z == bool4(true, false, true, false).xyz.z));
    }
    else
    {
        _59 = false;
    }
    bool _65 = false;
    if (_59)
    {
        bool4 _62 = bool4(!_40.x, !_40.y, !_40.z, !_40.w);
        _65 = all(bool4(_62.x == bool4(true, false, true, false).x, _62.y == bool4(true, false, true, false).y, _62.z == bool4(true, false, true, false).z, _62.w == bool4(true, false, true, false).w));
    }
    else
    {
        _65 = false;
    }
    bool _72 = false;
    if (_65)
    {
        _72 = all(bool2(bool2(true, false).x == bool4(true, false, true, false).xy.x, bool2(true, false).y == bool4(true, false, true, false).xy.y));
    }
    else
    {
        _72 = false;
    }
    bool _79 = false;
    if (_72)
    {
        _79 = all(bool3(bool3(true, false, true).x == bool4(true, false, true, false).xyz.x, bool3(true, false, true).y == bool4(true, false, true, false).xyz.y, bool3(true, false, true).z == bool4(true, false, true, false).xyz.z));
    }
    else
    {
        _79 = false;
    }
    bool _82 = false;
    if (_79)
    {
        _82 = true;
    }
    else
    {
        _82 = false;
    }
    float4 _83 = 0.0f.xxxx;
    if (_82)
    {
        _83 = _7_colorGreen;
    }
    else
    {
        _83 = _7_colorRed;
    }
    return _83;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
