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
    bool4 inputVal = bool4(_10_inputH4.x != 0.0f, _10_inputH4.y != 0.0f, _10_inputH4.z != 0.0f, _10_inputH4.w != 0.0f);
    bool4 expected = bool4(_10_expectedH4.x != 0.0f, _10_expectedH4.y != 0.0f, _10_expectedH4.z != 0.0f, _10_expectedH4.w != 0.0f);
    bool _73 = false;
    if (all(inputVal.xy) == expected.x)
    {
        _73 = all(inputVal.xyz) == expected.y;
    }
    else
    {
        _73 = false;
    }
    bool _81 = false;
    if (_73)
    {
        _81 = all(inputVal) == expected.z;
    }
    else
    {
        _81 = false;
    }
    bool _86 = false;
    if (_81)
    {
        _86 = expected.x;
    }
    else
    {
        _86 = false;
    }
    bool _92 = false;
    if (_86)
    {
        _92 = false == expected.y;
    }
    else
    {
        _92 = false;
    }
    bool _98 = false;
    if (_92)
    {
        _98 = false == expected.z;
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
