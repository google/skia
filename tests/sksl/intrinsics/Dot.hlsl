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
    bool _60 = false;
    if ((_10_inputA.x * _10_inputB.x) == expected.x)
    {
        _60 = dot(_10_inputA.xy, _10_inputB.xy) == expected.y;
    }
    else
    {
        _60 = false;
    }
    bool _74 = false;
    if (_60)
    {
        _74 = dot(_10_inputA.xyz, _10_inputB.xyz) == expected.z;
    }
    else
    {
        _74 = false;
    }
    bool _85 = false;
    if (_74)
    {
        _85 = dot(_10_inputA, _10_inputB) == expected.w;
    }
    else
    {
        _85 = false;
    }
    bool _91 = false;
    if (_85)
    {
        _91 = 5.0f == expected.x;
    }
    else
    {
        _91 = false;
    }
    bool _97 = false;
    if (_91)
    {
        _97 = 17.0f == expected.y;
    }
    else
    {
        _97 = false;
    }
    bool _103 = false;
    if (_97)
    {
        _103 = 38.0f == expected.z;
    }
    else
    {
        _103 = false;
    }
    bool _109 = false;
    if (_103)
    {
        _109 = 70.0f == expected.w;
    }
    else
    {
        _109 = false;
    }
    float4 _110 = 0.0f.xxxx;
    if (_109)
    {
        _110 = _10_colorGreen;
    }
    else
    {
        _110 = _10_colorRed;
    }
    return _110;
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
