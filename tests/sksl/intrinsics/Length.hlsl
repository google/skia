cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_inputVal : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 expected = float4(3.0f, 3.0f, 5.0f, 13.0f);
    bool _48 = false;
    if (length(_10_inputVal.x) == 3.0f)
    {
        _48 = length(_10_inputVal.xy) == 3.0f;
    }
    else
    {
        _48 = false;
    }
    bool _57 = false;
    if (_48)
    {
        _57 = length(_10_inputVal.xyz) == 5.0f;
    }
    else
    {
        _57 = false;
    }
    bool _64 = false;
    if (_57)
    {
        _64 = length(_10_inputVal) == 13.0f;
    }
    else
    {
        _64 = false;
    }
    bool _68 = false;
    if (_64)
    {
        _68 = true;
    }
    else
    {
        _68 = false;
    }
    bool _71 = false;
    if (_68)
    {
        _71 = true;
    }
    else
    {
        _71 = false;
    }
    bool _74 = false;
    if (_71)
    {
        _74 = true;
    }
    else
    {
        _74 = false;
    }
    bool _77 = false;
    if (_74)
    {
        _77 = true;
    }
    else
    {
        _77 = false;
    }
    float4 _78 = 0.0f.xxxx;
    if (_77)
    {
        _78 = _10_colorGreen;
    }
    else
    {
        _78 = _10_colorRed;
    }
    return _78;
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
