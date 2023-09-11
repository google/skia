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
    bool4 _41 = bool4(_7_colorRed.xxzw.x != 0.0f, _7_colorRed.xxzw.y != 0.0f, _7_colorRed.xxzw.z != 0.0f, _7_colorRed.xxzw.w != 0.0f);
    bool4 inputVal = _41;
    bool4 _54 = bool4(_7_colorRed.xyzz.x != 0.0f, _7_colorRed.xyzz.y != 0.0f, _7_colorRed.xyzz.z != 0.0f, _7_colorRed.xyzz.w != 0.0f);
    bool4 expected = _54;
    bool _59 = _54.x;
    bool _68 = false;
    if (all(_41.xy) == _59)
    {
        _68 = all(_41.xyz) == _54.y;
    }
    else
    {
        _68 = false;
    }
    bool _74 = false;
    if (_68)
    {
        _74 = all(_41) == _54.z;
    }
    else
    {
        _74 = false;
    }
    bool _77 = false;
    if (_74)
    {
        _77 = _59;
    }
    else
    {
        _77 = false;
    }
    bool _82 = false;
    if (_77)
    {
        _82 = false == _54.y;
    }
    else
    {
        _82 = false;
    }
    bool _87 = false;
    if (_82)
    {
        _87 = false == _54.z;
    }
    else
    {
        _87 = false;
    }
    float4 _88 = 0.0f.xxxx;
    if (_87)
    {
        _88 = _7_colorGreen;
    }
    else
    {
        _88 = _7_colorRed;
    }
    return _88;
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
