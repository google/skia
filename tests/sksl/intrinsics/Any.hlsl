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
    bool4 _41 = bool4(_7_colorGreen.xxyz.x != 0.0f, _7_colorGreen.xxyz.y != 0.0f, _7_colorGreen.xxyz.z != 0.0f, _7_colorGreen.xxyz.w != 0.0f);
    bool4 inputVal = _41;
    bool4 _54 = bool4(_7_colorGreen.xyyw.x != 0.0f, _7_colorGreen.xyyw.y != 0.0f, _7_colorGreen.xyyw.z != 0.0f, _7_colorGreen.xyyw.w != 0.0f);
    bool4 expected = _54;
    bool _59 = _54.x;
    bool _68 = false;
    if (any(_41.xy) == _59)
    {
        _68 = any(_41.xyz) == _54.y;
    }
    else
    {
        _68 = false;
    }
    bool _74 = false;
    if (_68)
    {
        _74 = any(_41) == _54.z;
    }
    else
    {
        _74 = false;
    }
    bool _78 = false;
    if (_74)
    {
        _78 = false == _59;
    }
    else
    {
        _78 = false;
    }
    bool _82 = false;
    if (_78)
    {
        _82 = _54.y;
    }
    else
    {
        _82 = false;
    }
    bool _86 = false;
    if (_82)
    {
        _86 = _54.z;
    }
    else
    {
        _86 = false;
    }
    float4 _87 = 0.0f.xxxx;
    if (_86)
    {
        _87 = _7_colorGreen;
    }
    else
    {
        _87 = _7_colorRed;
    }
    return _87;
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
