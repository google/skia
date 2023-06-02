cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool4 _42 = bool4(_10_colorGreen.x != 0.0f, _10_colorGreen.y != 0.0f, _10_colorGreen.z != 0.0f, _10_colorGreen.w != 0.0f);
    bool4 inputVal = _42;
    bool4 expected = bool4(true, false, true, false);
    bool2 _48 = _42.xy;
    bool2 _47 = bool2(!_48.x, !_48.y);
    bool _61 = false;
    if (all(bool2(_47.x == bool4(true, false, true, false).xy.x, _47.y == bool4(true, false, true, false).xy.y)))
    {
        bool3 _56 = _42.xyz;
        bool3 _55 = bool3(!_56.x, !_56.y, !_56.z);
        _61 = all(bool3(_55.x == bool4(true, false, true, false).xyz.x, _55.y == bool4(true, false, true, false).xyz.y, _55.z == bool4(true, false, true, false).xyz.z));
    }
    else
    {
        _61 = false;
    }
    bool _67 = false;
    if (_61)
    {
        bool4 _64 = bool4(!_42.x, !_42.y, !_42.z, !_42.w);
        _67 = all(bool4(_64.x == bool4(true, false, true, false).x, _64.y == bool4(true, false, true, false).y, _64.z == bool4(true, false, true, false).z, _64.w == bool4(true, false, true, false).w));
    }
    else
    {
        _67 = false;
    }
    bool _74 = false;
    if (_67)
    {
        _74 = all(bool2(bool2(true, false).x == bool4(true, false, true, false).xy.x, bool2(true, false).y == bool4(true, false, true, false).xy.y));
    }
    else
    {
        _74 = false;
    }
    bool _81 = false;
    if (_74)
    {
        _81 = all(bool3(bool3(true, false, true).x == bool4(true, false, true, false).xyz.x, bool3(true, false, true).y == bool4(true, false, true, false).xyz.y, bool3(true, false, true).z == bool4(true, false, true, false).xyz.z));
    }
    else
    {
        _81 = false;
    }
    bool _84 = false;
    if (_81)
    {
        _84 = true;
    }
    else
    {
        _84 = false;
    }
    float4 _85 = 0.0f.xxxx;
    if (_84)
    {
        _85 = _10_colorGreen;
    }
    else
    {
        _85 = _10_colorRed;
    }
    return _85;
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
