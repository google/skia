cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_inputA : packoffset(c0);
    float4 _10_inputB : packoffset(c1);
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
    float4 expected = float4(5.0f, 17.0f, 38.0f, 70.0f);
    bool _56 = false;
    if ((_10_inputA.x * _10_inputB.x) == 5.0f)
    {
        _56 = dot(_10_inputA.xy, _10_inputB.xy) == 17.0f;
    }
    else
    {
        _56 = false;
    }
    bool _68 = false;
    if (_56)
    {
        _68 = dot(_10_inputA.xyz, _10_inputB.xyz) == 38.0f;
    }
    else
    {
        _68 = false;
    }
    bool _77 = false;
    if (_68)
    {
        _77 = dot(_10_inputA, _10_inputB) == 70.0f;
    }
    else
    {
        _77 = false;
    }
    bool _81 = false;
    if (_77)
    {
        _81 = true;
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
    bool _87 = false;
    if (_84)
    {
        _87 = true;
    }
    else
    {
        _87 = false;
    }
    bool _90 = false;
    if (_87)
    {
        _90 = true;
    }
    else
    {
        _90 = false;
    }
    float4 _91 = 0.0f.xxxx;
    if (_90)
    {
        _91 = _10_colorGreen;
    }
    else
    {
        _91 = _10_colorRed;
    }
    return _91;
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
