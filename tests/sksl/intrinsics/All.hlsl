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
    bool4 _43 = bool4(_10_colorRed.xxzw.x != 0.0f, _10_colorRed.xxzw.y != 0.0f, _10_colorRed.xxzw.z != 0.0f, _10_colorRed.xxzw.w != 0.0f);
    bool4 inputVal = _43;
    bool4 _56 = bool4(_10_colorRed.xyzz.x != 0.0f, _10_colorRed.xyzz.y != 0.0f, _10_colorRed.xyzz.z != 0.0f, _10_colorRed.xyzz.w != 0.0f);
    bool4 expected = _56;
    bool _61 = _56.x;
    bool _70 = false;
    if (all(_43.xy) == _61)
    {
        _70 = all(_43.xyz) == _56.y;
    }
    else
    {
        _70 = false;
    }
    bool _76 = false;
    if (_70)
    {
        _76 = all(_43) == _56.z;
    }
    else
    {
        _76 = false;
    }
    bool _79 = false;
    if (_76)
    {
        _79 = _61;
    }
    else
    {
        _79 = false;
    }
    bool _84 = false;
    if (_79)
    {
        _84 = false == _56.y;
    }
    else
    {
        _84 = false;
    }
    bool _89 = false;
    if (_84)
    {
        _89 = false == _56.z;
    }
    else
    {
        _89 = false;
    }
    float4 _90 = 0.0f.xxxx;
    if (_89)
    {
        _90 = _10_colorGreen;
    }
    else
    {
        _90 = _10_colorRed;
    }
    return _90;
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
