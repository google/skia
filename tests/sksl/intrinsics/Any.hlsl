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
    bool4 _43 = bool4(_10_colorGreen.xxyz.x != 0.0f, _10_colorGreen.xxyz.y != 0.0f, _10_colorGreen.xxyz.z != 0.0f, _10_colorGreen.xxyz.w != 0.0f);
    bool4 inputVal = _43;
    bool4 _56 = bool4(_10_colorGreen.xyyw.x != 0.0f, _10_colorGreen.xyyw.y != 0.0f, _10_colorGreen.xyyw.z != 0.0f, _10_colorGreen.xyyw.w != 0.0f);
    bool4 expected = _56;
    bool _61 = _56.x;
    bool _70 = false;
    if (any(_43.xy) == _61)
    {
        _70 = any(_43.xyz) == _56.y;
    }
    else
    {
        _70 = false;
    }
    bool _76 = false;
    if (_70)
    {
        _76 = any(_43) == _56.z;
    }
    else
    {
        _76 = false;
    }
    bool _80 = false;
    if (_76)
    {
        _80 = false == _61;
    }
    else
    {
        _80 = false;
    }
    bool _84 = false;
    if (_80)
    {
        _84 = _56.y;
    }
    else
    {
        _84 = false;
    }
    bool _88 = false;
    if (_84)
    {
        _88 = _56.z;
    }
    else
    {
        _88 = false;
    }
    float4 _89 = 0.0f.xxxx;
    if (_88)
    {
        _89 = _10_colorGreen;
    }
    else
    {
        _89 = _10_colorRed;
    }
    return _89;
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
