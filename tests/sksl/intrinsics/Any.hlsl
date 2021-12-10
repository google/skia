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
    bool _60 = _55.x;
    bool _69 = false;
    if (any(_42.xy) == _60)
    {
        _69 = any(_42.xyz) == _55.y;
    }
    else
    {
        _69 = false;
    }
    bool _75 = false;
    if (_69)
    {
        _75 = any(_42) == _55.z;
    }
    else
    {
        _75 = false;
    }
    bool _79 = false;
    if (_75)
    {
        _79 = false == _60;
    }
    else
    {
        _79 = false;
    }
    bool _83 = false;
    if (_79)
    {
        _83 = _55.y;
    }
    else
    {
        _83 = false;
    }
    bool _87 = false;
    if (_83)
    {
        _87 = _55.z;
    }
    else
    {
        _87 = false;
    }
    float4 _88 = 0.0f.xxxx;
    if (_87)
    {
        _88 = _10_colorGreen;
    }
    else
    {
        _88 = _10_colorRed;
    }
    return _88;
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
