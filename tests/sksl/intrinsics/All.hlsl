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
    if (all(_42.xy) == _60)
    {
        _69 = all(_42.xyz) == _55.y;
    }
    else
    {
        _69 = false;
    }
    bool _75 = false;
    if (_69)
    {
        _75 = all(_42) == _55.z;
    }
    else
    {
        _75 = false;
    }
    bool _78 = false;
    if (_75)
    {
        _78 = _60;
    }
    else
    {
        _78 = false;
    }
    bool _83 = false;
    if (_78)
    {
        _83 = false == _55.y;
    }
    else
    {
        _83 = false;
    }
    bool _88 = false;
    if (_83)
    {
        _88 = false == _55.z;
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
