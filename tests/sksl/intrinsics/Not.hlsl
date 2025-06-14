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

float4 main(float2 _25)
{
    bool4 _43 = bool4(_11_colorGreen.x != 0.0f, _11_colorGreen.y != 0.0f, _11_colorGreen.z != 0.0f, _11_colorGreen.w != 0.0f);
    bool4 inputVal = _43;
    bool4 expected = bool4(true, false, true, false);
    bool2 _49 = _43.xy;
    bool2 _48 = bool2(!_49.x, !_49.y);
    bool _62 = false;
    if (all(bool2(_48.x == bool4(true, false, true, false).xy.x, _48.y == bool4(true, false, true, false).xy.y)))
    {
        bool3 _57 = _43.xyz;
        bool3 _56 = bool3(!_57.x, !_57.y, !_57.z);
        _62 = all(bool3(_56.x == bool4(true, false, true, false).xyz.x, _56.y == bool4(true, false, true, false).xyz.y, _56.z == bool4(true, false, true, false).xyz.z));
    }
    else
    {
        _62 = false;
    }
    bool _68 = false;
    if (_62)
    {
        bool4 _65 = bool4(!_43.x, !_43.y, !_43.z, !_43.w);
        _68 = all(bool4(_65.x == bool4(true, false, true, false).x, _65.y == bool4(true, false, true, false).y, _65.z == bool4(true, false, true, false).z, _65.w == bool4(true, false, true, false).w));
    }
    else
    {
        _68 = false;
    }
    bool _75 = false;
    if (_68)
    {
        _75 = all(bool2(bool2(true, false).x == bool4(true, false, true, false).xy.x, bool2(true, false).y == bool4(true, false, true, false).xy.y));
    }
    else
    {
        _75 = false;
    }
    bool _82 = false;
    if (_75)
    {
        _82 = all(bool3(bool3(true, false, true).x == bool4(true, false, true, false).xyz.x, bool3(true, false, true).y == bool4(true, false, true, false).xyz.y, bool3(true, false, true).z == bool4(true, false, true, false).xyz.z));
    }
    else
    {
        _82 = false;
    }
    bool _85 = false;
    if (_82)
    {
        _85 = true;
    }
    else
    {
        _85 = false;
    }
    float4 _86 = 0.0f.xxxx;
    if (_85)
    {
        _86 = _11_colorGreen;
    }
    else
    {
        _86 = _11_colorRed;
    }
    return _86;
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
